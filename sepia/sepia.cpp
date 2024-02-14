#include <CL/sycl.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace cl::sycl;


// Function to apply sepia effect using SYCL
void applySepiaKernelSYCL(queue q, unsigned char* input, unsigned char* output, size_t sizeX, size_t sizeY) {
	// Sepia transformation matrix
	const float sepia[3][3] = {
		{0.393f, 0.769f, 0.189f},
		{0.349f, 0.686f, 0.168f},
		{0.272f, 0.534f, 0.131f}
	};

	// ToDo
}

// Function to apply sepia effect to an image
void applySepiaSYCL(queue q, Mat& input, unsigned char* inputPtrUSM, Mat& output, unsigned char* outputPtrUSM) {
	output = Mat::zeros(input.size(), input.type());

	// Prepare input and output pointers
	size_t sizeX = input.cols;
	size_t sizeY = input.rows;

	unsigned char* inputPtr_host  = reinterpret_cast<unsigned char*>(input.data);
	unsigned char* outputPtr_host = reinterpret_cast<unsigned char*>(output.data);

	// To fill with q.memcpy(...).wait(); Note: host2device copy

	// Apply sepia effect using applySepiaKernel
	applySepiaKernelSYCL(q, inputPtrUSM, outputPtrUSM, sizeX, sizeY);
	q.wait();

	// To fill with q.memcpy(...).wait(); Note: device2host copy
}

// Function to apply sepia effect using a kernel
void applySepiaKernel(unsigned char* input, unsigned char* output, size_t sizeX, size_t sizeY) {
	// Sepia transformation matrix
	float sepia[3][3] = {
		{0.393f, 0.769f, 0.189f},
		{0.349f, 0.686f, 0.168f},
		{0.272f, 0.534f, 0.131f}
	};

	for (int y = 0; y < sizeY; ++y) {
		for (int x = 0; x < sizeX; ++x) {
			float r_in = input[(y * sizeX + x) * 3 + 2]; // Red channel
			float g_in = input[(y * sizeX + x) * 3 + 1]; // Green channel
			float b_in = input[(y * sizeX + x) * 3 + 0]; // Blue channel

			float r_out = sepia[0][0]*r_in + sepia[0][1]*g_in + sepia[0][2]*b_in;
			float g_out = sepia[1][0]*r_in + sepia[1][1]*g_in + sepia[1][2]*b_in;
			float b_out = sepia[2][0]*r_in + sepia[2][1]*g_in + sepia[2][2]*b_in;

			// Clamp the values to be within the range [0, 255]
			r_out = std::min(std::max(r_out, 0.0f), 255.0f);
			g_out = std::min(std::max(g_out, 0.0f), 255.0f);
			b_out = std::min(std::max(b_out, 0.0f), 255.0f);

			// Set the output pixel
			output[(y * sizeX + x) * 3 + 0] = b_out;
			output[(y * sizeX + x) * 3 + 1] = g_out;
			output[(y * sizeX + x) * 3 + 2] = r_out;
		}
	}
}

// Function to apply sepia effect to an image
void applySepia(Mat& input, Mat& output) {
	output = Mat::zeros(input.size(), input.type());

	// Prepare input and output pointers
	size_t sizeX = input.cols;
	size_t sizeY = input.rows;
	unsigned char* inputPtr  = reinterpret_cast<unsigned char*>(input.data);
	unsigned char* outputPtr = reinterpret_cast<unsigned char*>(output.data);

	// Apply sepia effect using applySepiaKernel
	applySepiaKernel(inputPtr, outputPtr, sizeX, sizeY);
}


int main(int argc, char** argv) {
	// Check for command line argument
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <video_filename|0 for webcam>" << std::endl;
		return 1;
	}

	// Initialize based on argument
	bool use_webcam = (std::string(argv[1]) == "0");
	VideoCapture cap;
	if (use_webcam) {
		cap = VideoCapture(0);
	} else {
		cap = VideoCapture(argv[1]);
	}

	// Print device information
	queue q(sycl::default_selector_v); //To fill with device
	device dev = q.get_device();
	std::cout << "Device Name: " << dev.get_info<info::device::name>() << std::endl;
	bool exec_SYCL = false;
	
	// Allocate memory using USM
	Mat frame;
	cap >> frame;
	Size frameSize = frame.size();
	int frameWidth = frameSize.width;
	int frameHeight = frameSize.height;
	
	unsigned char* inputUSM;  // To fill with malloc_shared. Note that frame has 3*frameWidth*frameHeight size
	unsigned char* outputUSM; // To fill with malloc_shared.

	// Check if the webcam is opened successfully
	if (!cap.isOpened()) {
		std::cout << "Error: Could not open video" << std::endl;
		return -1;
	}
  
	while (true) {
		// Capture a frame from the video
		Mat frame;
		cap >> frame;
		
		// Check if the frame is captured successfully
		if (frame.empty()) {
			std::cout << "Error: Could not capture frame" << std::endl;
			break;
		}

		// Apply sepia effect to the frame
		Mat sepiaFrame;
		if(!exec_SYCL){
			applySepia(frame, sepiaFrame);
		} else {
			applySepiaSYCL(q, frame, inputUSM, sepiaFrame, outputUSM);
		}
		
		// Create a new frame by horizontally concatenating the original frame and the sepia frame
		Mat combinedFrame;
		hconcat(frame, sepiaFrame, combinedFrame);

		String windowName;
		if(!exec_SYCL) windowName = "Original+Sepia Image (host)";
		else windowName = "Original+Sepia Image (SYCL)";
		
		imshow(windowName, combinedFrame);
		
		// Wait for a key press with a delay of 10 milliseconds
		int key = waitKey(10);

		// Process key press
		switch (key) {
		case 'q':
			// If 'q' key is pressed, exit the loop
			break;
		case 's':
			// If 's' key is pressed, save the frame to a file
			imwrite("frame.jpg", combinedFrame);
			std::cout << "Frame saved as frame.jpg" << std::endl;
			break;
		case 'c':
			// If 'c' key is pressed, perform action in host
			exec_SYCL = false;
			break;
		case 'd':
			// If 'c' key is pressed, perform action in device by means of SYCL
			exec_SYCL = true;
			break;

		default:
			// If any other key is pressed, do nothing
			break;
		}
    
		// Check if 'q' key is pressed, and exit the loop if true
		if (key == 'q') {
			break;
		}
		
		// Check for q key press to exit
		if (waitKey(1) == 'q') {
			break;
		}
	}

	// Release resources
	cap.release();
	return 0;
}
