import cv2
import numpy as np
import time
import serial
import struct

# --- 从新文件中导入卡尔曼滤波器类 ---
from kalman_filter_module import TrackerKalmanFilter



def create_signed_packet(num1, num2, endian='big'):
    """ (函数内容保持不变) """
    FRAME_HEADER = b'\x0A'
    FRAME_TAIL = b'\x55'
    if not (-32768 <= num1 <= 32767 and -32768 <= num2 <= 32767):
        raise ValueError("数字必须在 -32768 到 32767 范围内")
    if endian == 'big':
        format_string = '>hh'
    elif endian == 'little':
        format_string = '<hh'
    else:
        raise ValueError("字节序必须是 'big' 或 'little'")
    data_payload = struct.pack(format_string, num1, num2)
    packet = FRAME_HEADER + data_payload + FRAME_TAIL
    return packet
    
def find_intersection(line1, line2):
    """ (函数内容保持不变) """
    (x1, y1), (x2, y2) = line1
    (x3, y3), (x4, y4) = line2
    den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
    if den == 0:
        return None
    t_num = (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)
    u_num = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3))
    t = t_num / den
    u = u_num / den
    if 0 <= t <= 1 and 0 <= u <= 1:
        intersect_x = int(x1 + t * (x2 - x1))
        intersect_y = int(y1 + t * (y2 - y1))      
        return (intersect_x, intersect_y)
    return None

def is_parallelogram(points, rel_tolerance=0.15):
    """ (函数内容保持不变) """
    if points.shape != (4, 2):
        return False
    p0, p1, p2, p3 = points
    midpoint1 = (p0 + p2) / 2
    midpoint2 = (p1 + p3) / 2
    diag1_len = np.linalg.norm(p0 - p2)
    diag2_len = np.linalg.norm(p1 - p3)
    if diag1_len == 0 or diag2_len == 0:
        return False
    avg_diag_len = (diag1_len + diag2_len) / 2
    midpoint_dist = np.linalg.norm(midpoint1 - midpoint2)
    return midpoint_dist < (avg_diag_len * rel_tolerance)


# --- 主程序入口 ---
if __name__ == "__main__":

    # ... [串口代码保持不变] ...

    MIN_CONTOUR_AREA = 20
    MAX_CONTOUR_AREA = 2000
    LENTH = 50


    previous_time = 0
    now_time = 0
    dt = 1 / 30.0 

    kalman_tracker = TrackerKalmanFilter(dt_init=dt)
    
    # "corrected_point" 是对 *当前帧* 的最终估计 (黄色方块)
    corrected_point = None 
    # "predicted_point" 是对 *下一帧* 的预测 (蓝色十字)
    predicted_point = None
    
    cap = cv2.VideoCapture(0) 
    
    if not cap.isOpened():
        print("错误：无法打开摄像头。")
        exit()

    # 串口设备名称，树莓派 5 上 /dev/serial0 通常会映射到 UART0
    port = '/dev/serial0'
    # 波特率（根据需要修改，常用 9600、115200 等）
    baudrate = 115200
    # 打开串口
    try:
        ser = serial.Serial(port=port,
                            baudrate=baudrate,
                            bytesize=serial.EIGHTBITS,
                            parity=serial.PARITY_NONE,
                            stopbits=serial.STOPBITS_ONE,
                            timeout=1)  # 读取超时 1 秒
    except serial.SerialException as e:
        print(f"无法打开串口 {port}: {e}")
 
    print(f"串口 {port} 已打开，波特率 {baudrate}")

    print("按 'q' 键退出...")

    while True:
        # --- 1. 计算时间差 (dt) ---
        now_time = previous_time
        previous_time = time.time()
        dt = previous_time - now_time
        # (避免启动时的 dt=0 或 dt 异常大)
        if dt <= 0 or dt > 0.5:
            dt = 1 / 30.0 # 恢复为默认 30fps

        ret, frame = cap.read()
        if not ret:
            print("无法接收帧，退出...")
            break

        # --- 2. 卡尔曼 "先验" 预测 (k-1 -> k) ---
        # 这会推进滤波器的内部状态, 得到对 *当前帧* 的初步预测
        apriori_point = kalman_tracker.predict(dt)
        
        # 默认情况下, 最终点 = 初步预测点
        # (如果本帧没有检测到目标, 这将是我们的最终输出)
        corrected_point = apriori_point
        
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        ret, thresh_img = cv2.threshold(gray, 240, 255, cv2.THRESH_BINARY)
        contours, _ = cv2.findContours(thresh_img.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        detected_centers = [] 
        measurement_found_this_frame = False
        intersection_point = None # 重置
        
        if contours:
            contours = sorted(contours, key=cv2.contourArea, reverse=True)
            top_contours = contours[:4] 
            
            for contour in top_contours:
                area = cv2.contourArea(contour)
                if MIN_CONTOUR_AREA < area < MAX_CONTOUR_AREA: 
                    M = cv2.moments(contour)
                    if M["m00"] != 0:
                        center_x = int(M["m10"] / M["m00"])
                        center_y = int(M["m01"] / M["m00"])
                        detected_centers.append((center_x, center_y))
                        cv2.drawContours(frame, [contour], -1, (0, 255, 0), 2)
                        
        if len(detected_centers) == 4:
            points = np.array(detected_centers, dtype=np.float32).reshape(-1, 1, 2)
            hull = cv2.convexHull(points, returnPoints=True)
            
            if hull is not None and len(hull) == 4:
                hull_points = hull.reshape(4, 2)
                
                if is_parallelogram(hull_points):
                    p0, p1, p2, p3 = hull_points[0], hull_points[1], hull_points[2], hull_points[3]
                    diag1 = (p0, p2)
                    diag2 = (p1, p3)
                    intersection_point = find_intersection(diag1, diag2)
                    
        # --- 3. 卡尔曼 "后验" 修正 (k) ---
        if intersection_point:
            measurement_found_this_frame = True
            
            # (绘制 红色十字: 原始测量值)
            cv2.drawMarker(frame, intersection_point, (0, 0, 255),
                           markerType=cv2.MARKER_CROSS, markerSize=25, thickness=2)

            # 我们有了一个有效的测量值, 用它来 "修正" 我们的初步预测
            corrected_point = kalman_tracker.correct(intersection_point)
            
        # --- 4. 真正的 "未来" 预测 (k -> k+1) ---
        # *在*修正之后, 我们使用 *最新* 的速度和位置
        # 来预测 *下一帧* (k+1) 会在哪里
        # 我们假设下一帧的 dt 和当前帧 dt 相同
        predicted_point = kalman_tracker.get_future_prediction(8*dt) 

        
        # --- 5.1 绘制结果 ---
        
        # (绘制 黄色方块: *当前帧* 的最终平滑位置)
        if corrected_point is not None:
            ser.write(create_signed_packet(corrected_point[0], corrected_point[1]))
            cv2.drawMarker(frame, corrected_point, (0, 255, 255),
                           markerType=cv2.MARKER_SQUARE, markerSize=20, thickness=2)

        # (绘制 蓝色十字: *下一帧* 的预测位置)
        if predicted_point is not None:

            cv2.drawMarker(frame, predicted_point, (255, 100, 0), 
                           markerType=cv2.MARKER_CROSS, markerSize=20, thickness=2)
            
        # --- 5.2 计算并绘制修正后的预测延长线交点 ---        
        fix_point = (0,0)
        if predicted_point is not None and corrected_point is not None and predicted_point[1] != corrected_point[1] and intersection_point is not None:
            k=(intersection_point[0] - predicted_point[0])/(predicted_point[1]-corrected_point[1])
        fix_point =  int(intersection_point[0] + k*(LENTH), int(intersection_point[1] + LENTH))
        cv2.drawMarker(frame, fix_point, (255, 0, 255),
                          markerType=cv2.MARKER_TILTED_CROSS, markerSize=20, thickness=2)
        
        # --- 6. 显示图像 ---
        if dt > 0:
            fps = 1.0 / dt
        else:
            fps = 0
        
        cv2.putText(frame, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 2)
        cv2.imshow('frame', frame)
        cv2.imshow('thresh_img', thresh_img) 

        if cv2.waitKey(1) == ord('q'):
            break

    # --- 7. 释放资源 ---
    cap.release()
    cv2.destroyAllWindows()