import cv2
import csv
import numpy as np
import argparse
from PIL import Image
import os
import time

previous_positions = {}
previous_time = time.time()

# 去除影子的函數
def remove_shadow(image):
    lab = cv2.cvtColor(image, cv2.COLOR_BGR2LAB)
    l, a, b = cv2.split(lab)
    _, l_thresh = cv2.threshold(l, 200, 255, cv2.THRESH_BINARY)
    kernel = np.ones((5, 5), np.uint8)
    l_thresh = cv2.morphologyEx(l_thresh, cv2.MORPH_CLOSE, kernel)
    return l_thresh

# 銳化函數
def sharpen(image):
    kernel = np.array([[0, -1, 0],
                       [-1, 5, -1],
                       [0, -1, 0]])
    return cv2.filter2D(image, -1, kernel)

# 開啟影片文件
cap = cv2.VideoCapture('Python/sports2.mp4')
# 建立背景減除器
fgbg = cv2.createBackgroundSubtractorMOG2()

imgName = 1

# 讀取影片的第一幀來取得影片資訊
ret, frame = cap.read()
frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

# 初始化影片寫入器
out = cv2.VideoWriter('output_final2.mp4', cv2.VideoWriter_fourcc(*'mp4v'), 10, (frame_width, frame_height))

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        print("Exiting")
        break

    # 應用背景減除器
    fgmask = fgbg.apply(frame)
    kernel = np.ones((5, 5), np.uint8)
    fgmask = cv2.morphologyEx(fgmask, cv2.MORPH_OPEN, kernel)
    fgmask = cv2.morphologyEx(fgmask, cv2.MORPH_CLOSE, kernel)

    # 去除影子
    shadow_removed = remove_shadow(frame)
    fgmask = cv2.bitwise_and(fgmask, shadow_removed)
    #連接組件標記
    num_labels, labels, stats, centroids = cv2.connectedComponentsWithStats(fgmask)

    # 繪製邊界和中心點
#    for i in range(1, num_labels):
#        x, y, w, h, area = stats[i]
#        if area > 300 and area <400:  # 過濾掉小面積物體
#            # 繪製矩形
#            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)


    fast_movement_threshold = 50  # 設定速度閾值
    contours, _ = cv2.findContours(fgmask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    for contour in contours:
        area = cv2.contourArea(contour)
        if 200 < area <= 300:
            # 假設這是羽毛球
            x, y, w, h = cv2.boundingRect(contour)
            current_time = time.time()
            current_position = (x + w // 2, y + h // 2)

            if 'ball' in previous_positions:
                prev_position = previous_positions['ball']
                dt = current_time - previous_time
                dx = current_position[0] - prev_position[0]
                dy = current_position[1] - prev_position[1]
                if dt>0:
                    speed = ((dx ** 2 + dy ** 2) ** 0.5) / dt
                    if speed > fast_movement_threshold:
                        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 0, 255), 2)  # 紅色框表示快速移動

            previous_positions['ball'] = current_position
            previous_time = current_time
            
            # 銳化羽毛球區域
            roi = frame[y:y+h, x:x+w]
            sharp_roi = sharpen(roi)
            frame[y:y+h, x:x+w] = sharp_roi

        #elif area > 1000:
        #    # 假設這是球拍或球員
        #    x, y, w, h = cv2.boundingRect(contour)
        #    # 銳化球拍或球員區域
        #    roi = frame[y:y+h, x:x+w]
        #    sharp_roi = sharpen(roi)
        #    frame[y:y+h, x:x+w] = sharp_roi
        #    cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)

    # 顯示結果
    cv2.imshow('Frame', frame)

    # 保存每一幀圖像
    cv2.imwrite(f'./result/{imgName}.jpg', frame)
    imgName += 1

    # 寫入影片
    out.write(frame)

    if cv2.waitKey(30) & 0xFF == ord('q'):
        break

cap.release()
out.release()

# 合併圖像回影片
img = cv2.imread('./result/1.jpg')
imageInfo = img.shape
size = (imageInfo[1], imageInfo[0])

videoWrite = cv2.VideoWriter('output_fina2.mp4', cv2.VideoWriter_fourcc(*'mp4v'), 10, size)

for i in range(1, imgName):
    if i % 3 == 0:
        img = cv2.imread(f'./result/{i}.jpg')
        videoWrite.write(img)

print(os.path.exists('output_fina2.mp4'))  # 應該返回 True
videoWrite.release()
cv2.destroyAllWindows()
