import cv2
import Car
import sys
import time

def main():


    video = cv2.VideoCapture(sys.argv[1])
    car = Car.Car()
    try:
        i = 0
        while video.isOpened():
            _, frame = video.read()
            if frame is None:
                video = cv2.VideoCapture(sys.argv[1])
                _, frame = video.read()
            print('frame %s' % i )

            car.Drive(frame)
            car.ShowCamera(1)
            time.sleep(0.01)


            i += 1
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        video.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
