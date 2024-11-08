import cv2
import socket
import numpy as np

# Define the UDP server details
udp_ip = "127.0.0.1"  # Replace with the C++ client's IP address if on a different machine
udp_port = 12345      # Make sure this matches the C++ client port

# Initialize the socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Open the webcam
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Error: Could not open webcam.")
    exit()

# Set up parameters for the video stream
encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]  # Adjust quality if needed

try:
    while True:
        # Capture frame-by-frame
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame.")
            break

        # Encode frame as JPEG (or H.264 equivalent if OpenCV supports it)
        # Using JPEG encoding here to reduce packet size and for compatibility
        result, encoded_frame = cv2.imencode('.jpg', frame, encode_param)

        if result:
            # Convert encoded image to bytes
            data = encoded_frame.tobytes()

            # Send the encoded frame over UDP
            sock.sendto(data, (udp_ip, udp_port))

        # Optional: Display the sending frame locally
        cv2.imshow("Webcam Stream", frame)
        if cv2.waitKey(1) == 27:  # Press 'ESC' to exit
            break

finally:
    # Release the webcam and close any open windows
    cap.release()
    cv2.destroyAllWindows()
    sock.close()
