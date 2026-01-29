import subprocess
import os
import win32gui, win32com.client, win32con
import pyautogui
import threading
import time
import psutil

def screenshot(windowTitle, imageName, offsetX, offsetY, reduceX, reduceY):
    global isInError
    if windowTitle:
        hwnd = win32gui.FindWindow(None, windowTitle)
        if hwnd:
            # Idk why but these 2 lines are needed (https://stackoverflow.com/questions/14295337/win32gui-setactivewindow-error-the-specified-procedure-could-not-be-found)
            shell = win32com.client.Dispatch("WScript.Shell")
            shell.SendKeys('%')
            win32gui.SetForegroundWindow(hwnd)
            win32gui.SetWindowPos(hwnd, win32con.HWND_TOPMOST, 0, 0, 0, 0, win32con.SWP_NOSIZE)
            time.sleep(0.1)
            x, y, x1, y1 = win32gui.GetClientRect(hwnd)
            x, y = win32gui.ClientToScreen(hwnd, (x, y))
            x1, y1 = win32gui.ClientToScreen(hwnd, (x1 - x, y1 - y))
            im = pyautogui.screenshot(region=(x + offsetX, y + offsetY, x1 - reduceX, y1 - reduceY))

            im.save(imageName)

            isInError = False
        else:
            isInError = True

