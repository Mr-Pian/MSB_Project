import cv2
import numpy as np

# --- 1. 定义常量和参数 ---

# 字典类型，必须与生成时一致
ARUCO_DICT_TYPE = cv2.aruco.DICT_5X5_50

# 标记的物理尺寸（单位：米）
MARKER_LENGTH_METERS = 0.05

# (新增) 定义我们希望还原的“正视”图像的尺寸（像素）
WARPED_IMG_WIDTH = 200
WARPED_IMG_HEIGHT = 200

# --- 2. 相机内参和畸变系数 ---
# ... (这部分代码保持不变) ...
focal_length = 500
camera_center_x = 640 / 2
camera_center_y = 480 / 2

camera_matrix = np.array(
    [
        [focal_length, 0, camera_center_x],
        [0, focal_length, camera_center_y],
        [0, 0, 1]
    ],
    dtype=np.float32
)
dist_coeffs = np.zeros((5, 1), dtype=np.float32)

print("使用的相机矩阵:\n", camera_matrix)
print("使用的畸变系数:\n", dist_coeffs)

# --- 3. 初始化ArUco检测器 ---

dictionary = cv2.aruco.getPredefinedDictionary(ARUCO_DICT_TYPE)
parameters = cv2.aruco.DetectorParameters()
detector = cv2.aruco.ArucoDetector(dictionary, parameters)

# --- (新增) 3.5 定义逆透视的目标(dst)坐标 ---
# 这4个点定义了我们希望得到的“正视”图像的四个角
# ArUco返回的角点顺序是：[左上, 右上, 右下, 左下]
dst_pts = np.array(
    [
        [0, 0],  # 左上
        [WARPED_IMG_WIDTH - 1, 0],  # 右上
        [WARPED_IMG_WIDTH - 1, WARPED_IMG_HEIGHT - 1],  # 右下
        [0, WARPED_IMG_HEIGHT - 1]  # 左下
    ],
    dtype=np.float32
)

# (新增) 创建一个空白画布，用于在未检测到标记时显示
warped_canvas = np.zeros((WARPED_IMG_WIDTH, WARPED_IMG_HEIGHT, 3), dtype=np.uint8)

# --- 4. 初始化摄像头 ---

cap = cv2.VideoCapture(1)
if not cap.isOpened():
    print("错误：无法打开摄像头。")
    exit()

print("\n摄像头已启动。将ArUco标记对准摄像头。")
print("按 'q' 键退出程序。")
cap.set(cv2.CAP_PROP_AUTO_WB, 0)
current_awb = cap.get(cv2.CAP_PROP_AUTO_WB)
print(f"当前自动白平衡设置: {current_awb}")

# --- 5. 循环处理视频帧 ---

while True:
    ret, frame = cap.read()
    if not ret:
        print("错误：无法读取视频帧。")
        break

    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    corners, ids, _ = detector.detectMarkers(gray_frame)
    
    # (新增) 默认将逆透视图像设置为空白画布
    warped_display_img = warped_canvas

    if ids is not None:
        cv2.aruco.drawDetectedMarkers(frame, corners, ids)

        # --- (新增) 开始执行逆透视变换 ---
        # 我们只对检测到的第一个标记(ids[0])进行逆透视
        
        # 1. 获取源(src)坐标点，即标记在当前帧中的4个角点
        #    确保数据类型为 float32
        src_pts = corners[0].astype(np.float32)

        # 2. 计算透视变换矩阵 M
        M = cv2.getPerspectiveTransform(src_pts, dst_pts)

        # 3. 应用透视变换，得到“正视”图像
        warped_img = cv2.warpPerspective(
            frame, M, (WARPED_IMG_WIDTH, WARPED_IMG_HEIGHT)
        )
        
        # (新增) 将计算得到的图像赋给显示变量
        warped_display_img = warped_img
        # --- (新增) 逆透视变换结束 ---

        # 估计姿态（这部分代码保持不变，用于在主窗口显示坐标轴）
        rvecs, tvecs, _ = cv2.aruco.estimatePoseSingleMarkers(
            corners, MARKER_LENGTH_METERS, camera_matrix, dist_coeffs
        )

        for i in range(len(ids)):
            rvec = rvecs[i]
            tvec = tvecs[i]
            cv2.drawFrameAxes(frame, camera_matrix, dist_coeffs, rvec, tvec, 0.03)

    # 显示结果
    cv2.imshow('ArUco Pose Estimation', frame)

    # (新增) 在一个新窗口中显示逆透视的结果
    cv2.imshow('Warped (Front View)', warped_display_img)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# --- 6. 释放资源 ---
print("正在关闭程序...")
cap.release()
cv2.destroyAllWindows()