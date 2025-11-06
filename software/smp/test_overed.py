import cv2
import numpy as np
import time
import serial
import struct

def create_signed_packet(num1, num2, endian='big'):
    """
    将两个 2 字节的 *带符号* 数字打包成一个6字节的定长数据包。
    
    :param num1: 第一个数字 (-32768 到 32767)
    :param num2: 第二个数字 (-32768 到 32767)
    :param endian: 字节序, 'big' (大端) 或 'little' (小端)
    :return: 6字节的 bytes 对象
    """

    FRAME_HEADER = b'\x0A'
    FRAME_TAIL = b'\x55'
    
    # 检查数字是否在 2 字节 *带符号* 整数范围内
    if not (-32768 <= num1 <= 32767 and -32768 <= num2 <= 32767):
        raise ValueError("数字必须在 -32768 到 32767 范围内")

    # 'h' 代表 2 字节的 *带符号* 整数 (signed short)
    # '>' 代表 大端 (Big-Endian)
    # '<' 代表 小端 (Little-Endian)
    
    if endian == 'big':
        format_string = '>hh' # 使用 'h'
    elif endian == 'little':
        format_string = '<hh' # 使用 'h'
    else:
        raise ValueError("字节序必须是 'big' 或 'little'")
        
    # 1. 将两个数字打包成 4 字节的数据
    data_payload = struct.pack(format_string, num1, num2)
    
    # 2. 拼接成完整的数据包
    packet = FRAME_HEADER + data_payload + FRAME_TAIL
    
    return packet
    
def find_intersection(line1, line2):
    """
    计算两条线段 [ (x1,y1), (x2,y2) ] 和 [ (x3,y3), (x4,y4) ] 的交点。
    
    参数:
    line1: 包含线段1的两个端点的元组, 格式为 ( (x1, y1), (x2, y2) )
    line2: 包含线段2的两个端点的元组, 格式为 ( (x3, y3), (x4, y4) )
    
    返回:
    如果相交，返回交点的 (x, y) 整数元组。
    如果不相交（平行或交点在线段外），返回 None。
    """
    
    # 从输入中提取所有坐标
    (x1, y1), (x2, y2) = line1
    (x3, y3), (x4, y4) = line2

    # --- 1. 计算分母 ---
    # 这是基于线段参数方程的行列式
    # (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4)
    den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
    
    # 如果分母为 0, 表示两条线平行或共线
    if den == 0:
        return None

    # --- 2. 计算分子 (t 和 u) ---
    # t 是交点在线段1上的位置比例 (0 <= t <= 1)
    # u 是交点在线段2上的位置比例 (0 <= u <= 1)
    
    t_num = (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)
    u_num = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3))

    # --- 3. 计算 t 和 u ---
    t = t_num / den
    u = u_num / den

    # --- 4. 检查交点是否在线段上 ---
    # 只有当 t 和 u 都在 [0, 1] 范围内时, 交点才同时位于两条线段上
    if 0 <= t <= 1 and 0 <= u <= 1:
        # 计算交点坐标
        intersect_x = int(x1 + t * (x2 - x1))
        intersect_y = int(y1 + t * (y2 - y1))      
        
        # (使用线段2和u计算也可以得到相同的结果)
        # intersect_x = int(x3 + u * (x4 - x3))
        # intersect_y = int(y3 + u * (y4 - y3))
        
        return (intersect_x, intersect_y)

    # 如果 t 或 u 不在 [0, 1] 范围内, 表示交点在线段的延长线上
    return None



# --- 主程序入口 ---
if __name__ == "__main__":


    # try:
    #     # 创建 Serial 对象，假设设备在 COM16，波特率为 115200
    #     # 使用 'with' 语句可以确保串口在使用后自动关闭
    #     ser = serial.Serial('COM24', 115200, timeout=1)
    #     print(f"已打开串口: {ser.name}")
            
    #         # 等待串口准备好
    #     time.sleep(2) 
            
    # except serial.SerialException as e:
    #     print(f"打开串口时出错: {e}")
    # except Exception as e:
    #     print(f"发生错误: {e}")




    previous_time = 0
    now_time = 0
    # --- 2. 打开摄像头 ---
    cap = cv2.VideoCapture(1) # 0 通常是你的默认摄像头



    if not cap.isOpened():
        print("错误：无法打开摄像头。")
        exit()

    print("按 'q' 键退出...")

    while True:
        now_time = previous_time
        # --- 3. 读取帧并转为灰度图 ---
        ret, frame = cap.read()
        if not ret:
            print("无法接收帧，退出...")
            break

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # --- 4. 预处理和检测 ---
        # 为了让 SimpleBlobDetector 的 color=255 生效，
        # 我们需要一个二值图像。我们使用阈值来分离出“白斑”。
        # 阈值 200 是一个示例，你需要根据你的光照条件调整！
        # 任何亮度 > 200 的像素会变成 255 (白色)，其余为 0 (黑色)
        # 这就是 SimpleBlobDetector 要处理的图像
        ret, thresh_img = cv2.threshold(gray, 240, 255, cv2.THRESH_BINARY)

        contours, _ = cv2.findContours(thresh_img.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        detected_centers = [] # 存储检测到的色块中心点

        # 遍历所有找到的轮廓
        if contours:
            # 按面积降序排序轮廓
            contours = sorted(contours, key=cv2.contourArea, reverse=True)
            
            # --- 6. 提取色块中心并绘制 ---
            # 我们只关心最大的 4 个轮廓
            top_contours = contours[:4] 
            
            for contour in top_contours:
                area = cv2.contourArea(contour)
                
                # 筛选轮廓：仅处理面积大于某个阈值的轮廓 (例如 100 像素)
                if area > 20 and area < 2000: 
                    # 计算轮廓的矩 (moments) 以找到中心点
                    M = cv2.moments(contour)
                    if M["m00"] != 0:
                        center_x = int(M["m10"] / M["m00"])
                        center_y = int(M["m01"] / M["m00"])
                        
                        detected_centers.append((center_x, center_y))

                        # 绘制白色块的轮廓 (绿色)
                        cv2.drawContours(frame, [contour], -1, (0, 255, 0), 2)
                        
        if len(detected_centers) == 4:
            # 将中心点转换为 numpy 数组，格式为 (N, 1, 2)
            points = np.array(detected_centers, dtype=np.float32).reshape(-1, 1, 2)
            
            # 1. 找到这 4 个点的凸包
            hull = cv2.convexHull(points, returnPoints=True)
            
            # 2. 确保凸包是一个四边形
            if hull is not None and len(hull) == 4:
                hull_points = hull.reshape(4, 2)
                p0, p1, p2, p3 = hull_points[0], hull_points[1], hull_points[2], hull_points[3]
                
                # 两个对角线
                diag1 = (p0, p2)
                diag2 = (p1, p3)
                
                # 3. 计算交点
                intersection_point = find_intersection(diag1, diag2)
                
                # 4. 如果找到交点，则标记
                if intersection_point:
                    marker_size = 25
                    color = (0, 0, 255) # 红色标记
                    thickness = 2

                    print("交点:", intersection_point)
                    
                    cv2.drawMarker(frame, 
                                intersection_point,
                                color,
                                markerType=cv2.MARKER_CROSS,
                                markerSize=marker_size,
                                thickness=thickness)

        # --- 6. 显示图像 ---
        previous_time = time.time()
        fps = -1/(now_time - previous_time) if (now_time - previous_time)!=0 else 0
        cv2.putText(frame, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 2)
        cv2.imshow('frame', frame)
        cv2.imshow('thresh_img', thresh_img) # 显示这个窗口有助于调试阈值

        # 按 'q' 退出
        if cv2.waitKey(1) == ord('q'):
            break

    # --- 7. 释放资源 ---
    cap.release()
    cv2.destroyAllWindows()