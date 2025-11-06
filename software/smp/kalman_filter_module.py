import cv2
import numpy as np

class TrackerKalmanFilter:
    """
    一个封装了 cv2.KalmanFilter 的追踪器类。
    它管理滤波器的状态、初始化和更新。
    """
    
    def __init__(self, dt_init=1/30.0):
        """
        初始化卡尔曼滤波器。
        :param dt_init: 初始的帧时间差 (delta-time)
        """
        # 状态: [x, y, dx, dy], 测量: [x, y]
        self.kalman = cv2.KalmanFilter(4, 2)
        
        # 测量矩阵 H
        self.kalman.measurementMatrix = np.array([[1, 0, 0, 0],
                                                  [0, 1, 0, 0]], dtype=np.float32)
        
        # 状态转移矩阵 F (dt 会在 predict 时更新)
        self.kalman.transitionMatrix = np.array([[1, 0, dt_init, 0],
                                                 [0, 1, 0, dt_init],
                                                 [0, 0, 1, 0],
                                                 [0, 0, 0, 1]], dtype=np.float32)
        
        # 过程噪声协方差 Q
        self.kalman.processNoiseCov = np.eye(4, dtype=np.float32) * 1e-4
        
        # 测量噪声协方差 R
        self.kalman.measurementNoiseCov = np.eye(2, dtype=np.float32) * 1e-1
        
        # 初始后验误差协方差 P
        self.kalman.errorCovPost = np.eye(4, dtype=np.float32) * 1.0
        
        # 状态标志
        self.is_kalman_initialized = False
        
        # 存储最新的预测点和修正点
        self.predicted_point = None
        self.corrected_point = None


    def predict(self, dt):
        """
        预测下一帧的位置。
        :param dt: 距离上一帧的时间差
        :return: (x, y) 预测点, or None
        """
        if not self.is_kalman_initialized:
            return None

        # 1. 更新 F 矩阵中的 dt
        self.kalman.transitionMatrix[0, 2] = dt
        self.kalman.transitionMatrix[1, 3] = dt
        
        # 2. 预测
        predicted_state = self.kalman.predict()
        self.predicted_point = (int(predicted_state[0, 0]), int(predicted_state[1, 0]))
        
        # 默认使用预测点, 除非稍后被测量值修正
        self.corrected_point = self.predicted_point
        return self.predicted_point


    def correct(self, intersection_point):
        """
        使用新的测量值修正滤波器。
        :param intersection_point: (x, y) 测量值
        :return: (x, y) 修正后的点
        """
        # 1. 准备测量值
        measurement = np.array([[intersection_point[0]], 
                                [intersection_point[1]]], dtype=np.float32)
        
        if not self.is_kalman_initialized:
            # 第一次初始化
            # 将初始状态设置为第一个测量值
            self.kalman.statePost = np.array([[intersection_point[0]], [intersection_point[1]], [0], [0]], dtype=np.float32)
            self.is_kalman_initialized = True
            self.corrected_point = intersection_point
            self.predicted_point = intersection_point # 初始时预测=修正
        else:
            # 修正 (使用测量值)
            corrected_state = self.kalman.correct(measurement)
            # 从修正状态中提取 (x, y)
            self.corrected_point = (int(corrected_state[0, 0]), int(corrected_state[1, 0]))
        
        return self.corrected_point
    
    
    def get_future_prediction(self, dt_future):
        """
        根据*最新*的修正后状态 (statePost)，计算未来的位置。
        这*不会*推进卡尔曼滤波器的内部状态，只是一个"查看"操作。

        :param dt_future: 您希望预测多远的未来时间 (例如，下一帧的 dt)
        :return: (x, y) 未来预测点, or None
        """
        if not self.is_kalman_initialized:
            return None
        
        # 1. 获取最新的"后验"状态 (即黄色方块代表的状态)
        # statePost 包含 [x, y, dx, dy]
        state = self.kalman.statePost 
        
        # 2. 创建一个临时的状态转移矩阵
        F_future = np.array([[1, 0, dt_future, 0],
                             [0, 1, 0, dt_future],
                             [0, 0, 1, 0],
                             [0, 0, 0, 1]], dtype=np.float32)
        
        # 3. 计算未来状态: state_future = F_future * state_post
        #    (注意: @ 是 numpy 的矩阵乘法)
        future_state = F_future @ state
        
        # 4. 返回未来状态的 (x, y)
        return (int(future_state[0, 0]), int(future_state[1, 0]))