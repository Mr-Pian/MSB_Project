import cv2
import numpy as np

# --- 1. 定义常量和参数 ---

# 字典类型，必须与生成时一致
ARUCO_DICT_TYPE = cv2.aruco.DICT_4X4_50

# 标记的物理尺寸（单位：米）
# 这是姿态估计的关键！你需要用尺子测量你打印出来的标记（黑色边框的边长）
# 并将这个值更新为实际测量结果。
# 例如，如果标记边长为5厘米，则此处应为 0.05
MARKER_LENGTH_METERS = 0.05

# --- 2. 相机内参和畸变系数（重要：请用你自己的校准数据替换！） ---
# 这是一个未经校准的示例值，假设摄像头分辨率为 640x480
# 对于实际应用，必须通过相机校准获得这些值
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

# 假设没有镜头畸变
dist_coeffs = np.zeros((5, 1), dtype=np.float32)

print("使用的相机矩阵:\n", camera_matrix)
print("使用的畸变系数:\n", dist_coeffs)

# --- 3. 初始化ArUco检测器 ---

# 获取预定义的ArUco字典
dictionary = cv2.aruco.getPredefinedDictionary(ARUCO_DICT_TYPE)

# 初始化ArUco检测器参数
parameters = cv2.aruco.DetectorParameters()

# 创建ArUco检测器
detector = cv2.aruco.ArucoDetector(dictionary, parameters)

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
    # 读取视频帧
    ret, frame = cap.read()
    if not ret:
        print("错误：无法读取视频帧。")
        break

    # 转换为灰度图
    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # 检测ArUco标记
    corners, ids, _ = detector.detectMarkers(gray_frame)

    # 如果检测到标记
    if ids is not None:
        # 绘制标记的边框和ID
        cv2.aruco.drawDetectedMarkers(frame, corners, ids)

        # 估计每个标记的姿态
        # rvecs: 旋转向量 (Rotation Vectors)
        # tvecs: 平移向量 (Translation Vectors)
        rvecs, tvecs, _ = cv2.aruco.estimatePoseSingleMarkers(
            corners, MARKER_LENGTH_METERS, camera_matrix, dist_coeffs
        )

        # 为每个检测到的标记绘制坐标轴
        for i in range(len(ids)):
            rvec = rvecs[i]
            tvec = tvecs[i]
            # 绘制坐标轴，最后一个参数是轴的长度（单位：米）
            cv2.drawFrameAxes(frame, camera_matrix, dist_coeffs, rvec, tvec, 0.03)

    # 显示结果
    cv2.imshow('ArUco Pose Estimation', frame)

    # 按 'q' 退出
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# --- 6. 释放资源 ---
print("正在关闭程序...")
cap.release()
cv2.destroyAllWindows()