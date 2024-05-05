#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat originalImage;
Mat currentImage;

bool isDrawing = false;
bool isDragging = false;

Point startPoint;
Point lastMousePos;
Rect viewportRect(500, 250, 1000, 500);

vector<Vec4i> lines;
vector<vector<Vec4i>> linesStack;
vector<vector<Vec4i>> deletedLinesStack;

void updateImage() {
	Mat viewport = currentImage(viewportRect);
	imshow("Drawing", viewport);
}

void controlZ() {
	if (linesStack.size() <= 0)
		return;

	vector<Vec4i> last = linesStack[linesStack.size() - 1];

	deletedLinesStack.push_back(last);

	linesStack.pop_back();
	currentImage = originalImage.clone();

	for (const auto& amountLines : linesStack) {
		for (const auto& oneLine : amountLines) {
			line(currentImage, Point(oneLine[0], oneLine[1]), Point(oneLine[2], oneLine[3]), Scalar(0, 0, 255), 2);
		}
	}

	updateImage();
}

void controlV() {

	if (linesStack.size() == 0 && deletedLinesStack.size() == 0)
		return;
	if (deletedLinesStack.size() == 0)
		return;

	vector<Vec4i> last = deletedLinesStack[deletedLinesStack.size() - 1];

	linesStack.push_back(last);
	deletedLinesStack.pop_back();

	currentImage = originalImage.clone();

	for (const auto& amountLines : linesStack) {
		for (const auto& oneLine : amountLines) {
			line(currentImage, Point(oneLine[0], oneLine[1]), Point(oneLine[2], oneLine[3]), Scalar(0, 0, 255), 2);
		}
	}

	updateImage();
}

static void drawCallback(int event, int x, int y, int flags, void* param) {
	switch (event) {
	case EVENT_LBUTTONDOWN:
		if (flags & EVENT_FLAG_SHIFTKEY) {
			isDragging = true;
			startPoint = Point(x, y);
			lastMousePos = startPoint;
		}
		else if (flags & EVENT_FLAG_CTRLKEY) {
			isDrawing = true;
			x += viewportRect.x;
			y += viewportRect.y;
			startPoint = Point(x, y);
		}

		break;
	case EVENT_MOUSEMOVE:
		if (isDrawing) {
			x += viewportRect.x;
			y += viewportRect.y;
			lines.push_back(Vec4i(startPoint.x, startPoint.y, x, y));
			line(currentImage, startPoint, Point(x, y), Scalar(0, 0, 255), 2);
			startPoint = Point(x, y);
			updateImage();
		}
		else if (isDragging) {
			int dx = x - lastMousePos.x;
			int dy = y - lastMousePos.y;

			viewportRect.x -= dx;
			viewportRect.y -= dy;

			if (viewportRect.x < 0)
				viewportRect.x = 0;

			if (viewportRect.y < 0)
				viewportRect.y = 0;

			lastMousePos = Point(x, y);
			if (viewportRect.x >= 0 && viewportRect.y >= 0)
				updateImage();
		}
		break;
	case EVENT_LBUTTONUP:
		if (isDragging) {
			x += viewportRect.x;
			y += viewportRect.y;
		}
		if (isDrawing)
			linesStack.push_back(lines);

		isDrawing = false;
		isDragging = false;
		lines.clear();
		break;
	}
}

int main()
{
	string imagePath = "C:/Users/Yuri/Pictures/space.jpg";

	originalImage = imread(imagePath, IMREAD_COLOR);

	currentImage = originalImage.clone();

	namedWindow("Drawing");

	updateImage();

	setMouseCallback("Drawing", drawCallback, nullptr);

	while (1) {
		imshow("Drawing", currentImage(viewportRect));
		int key = waitKey(0);
		cout << key << endl;
		if (key == 27)
			break;
		else if ((key & 0xFF) == 'z')
			controlZ();
		else if ((key & 0xFF) == 'v')
			controlV();
	}

	destroyAllWindows();
	return 0;
}



