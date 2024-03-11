import cv2
import numpy as np
import socket
import mss
import mss.tools
from win32api import GetKeyState
from win32con import VK_SHIFT
import win32gui
from PIL import ImageGrab
import time
import serial

serial_port = 'COM7'
baud_rate = 115200

ser = serial.Serial(serial_port, baud_rate, timeout=1)

time.sleep(2)

left = (1920/2)-25
top = (1080/2)-25
width, height = 50, 50

# monitor = {'top': int(top), 'left': int(left), 'width': int(left+width), 'height': int(top+height)}
screen_width = 1920
screen_height = 1080

square_size = 50
square_x = (screen_width - square_size) // 2
square_y = (screen_height - square_size) // 2
# Define the region to capture (100x100 square)
region = {
    "top": square_y,
    "left": square_x,
    "width": square_size,
    "height": square_size
}
def find_closest_purple_pixel(image):
    purple = (128, 0, 128)
    topmost_point = None
    min_y = float('inf')

    for y in range(image.shape[0]):
        for x in range(image.shape[1]):
            if all(image[y, x] == purple):
                if y < min_y:
                    min_y = y
                    topmost_point = (x, y)

    return topmost_point

def check_center_kernel_color(image, color_range):
    height, width, _ = image.shape

    kernel_size = 5
    half_kernel = kernel_size // 2
    center_x = width // 2
    center_y = height // 2

    for i in range(-half_kernel, half_kernel + 1):
        for j in range(-half_kernel, half_kernel + 1):
            x = center_x + i
            y = center_y + j

            if 0 <= x < width and 0 <= y < height:
                pixel_color = image[y, x]

                if np.all(pixel_color >= color_range[0]) and np.all(pixel_color <= color_range[1]):
                    return True

    return False

def detect_humanoid_figures_filled(image):
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    YellowLower = [0, 50, 50]
    YellowUpper = [10, 255, 255]

    lower_r = 140
    upper_r = 150
    PurpleLower = [lower_r, 75, 75]
    PurpleUpper = [upper_r, 255, 255]

    lower_purple = np.array(PurpleLower)
    upper_purple = np.array(PurpleUpper)

    mask = cv2.inRange(hsv, lower_purple, upper_purple)

    kernel = np.ones((3, 3), np.uint8)
    mask = cv2.dilate(mask, kernel, iterations=1)

    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    fill_mask = np.zeros_like(image)

    for contour in contours:
        cv2.fillPoly(fill_mask, [contour], (128, 0, 128))

    result = cv2.addWeighted(image, 1, fill_mask, 1, 0)
    
    closest_point = find_closest_purple_pixel(fill_mask)
    if closest_point is not None:
        center = (result.shape[1] // 2, result.shape[0] // 2)
        mF = 2.5
        mX = ((closest_point[0] - center[0]) + 1) * mF
        mY = (closest_point[1] - (center[1] - 2)) * mF
        
        ser.write((str(mX) + ":" + str(mY) + "x").encode())

        # if(check_center_kernel_color(result, (lower_purple, upper_purple))):
        #     ser.write(b'g')
    # cv2.imshow("fsdf", mask)
    return result


# prev_time = 0
with mss.mss() as sct:
    while True:
        # screenshot = ImageGrab.grab(bbox=(left, top, left+width, top+height))
        screenshot = sct.grab(region)

        frame = np.array(screenshot)

        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        final_res = detect_humanoid_figures_filled(frame_rgb)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            cv2.imwrite('./videos/capture.png', final_res)

        # print(1 / (time.time() - prev_time))
        # prev_time = time.time()

cv2.destroyAllWindows()
