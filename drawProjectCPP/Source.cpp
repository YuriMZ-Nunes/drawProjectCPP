#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat originalImage;
Mat currentImage;

bool isDrawing = false;
bool isDragging = false;
bool isZooming = false;
bool lockedDir = false;
bool lockedX = false;
bool lockedY = false;

Point startPoint;
Point lastMousePos;
Point lockStartPoint;
int lockX;
int lockY;

Rect viewportRect(5000, 2500, 1000, 500);

vector<Vec4i> lines;
vector<vector<Vec4i>> linesStack;
vector<vector<Vec4i>> deletedLinesStack;

/*TODO
*	- borracha apagar a linha toda
*	- criar formas
*/

void updateImage() {
	
	imshow("Drawing", currentImage(viewportRect));
	
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
			if (flags & EVENT_FLAG_CTRLKEY) {
				isDragging = true;
				startPoint = Point(x, y);
				lastMousePos = startPoint;
			}
			else {
				isDrawing = true;
				x += viewportRect.x;
				y += viewportRect.y;
				startPoint = Point(x, y);
			}
			break;
		case EVENT_MOUSEMOVE:
			if (isDrawing) {
				if (flags & EVENT_FLAG_SHIFTKEY) {
					if (!lockedDir) {
						lockStartPoint = startPoint;
						lockX = x;
						lockY = y;
						lockedDir = true;
					}

					int deltaX = x - lockX;
					int deltaY = y - lockY;

					if (deltaX < 0)
						deltaX *= -1;
					if (deltaY < 0)
						deltaY *= -1;

					if (deltaX > deltaY && !lockedY)
						lockedX = true;
					else if(deltaY > deltaX && !lockedX)
						lockedY = true;
					

					if (lockedX) {
						x += viewportRect.x;
						y = lockStartPoint.y;
						lines.push_back(Vec4i(startPoint.x, startPoint.y, x, y));
						line(currentImage, startPoint, Point(x, y), Scalar(0, 0, 255), 2);
						startPoint = Point(x, y);
					}
					else {
						x = lockStartPoint.x;
						y += viewportRect.y;
						lines.push_back(Vec4i(startPoint.x, startPoint.y, x, y));
						line(currentImage, startPoint, Point(x, y), Scalar(0, 0, 255), 2);
						startPoint = Point(x, y);
					}
					updateImage();
				}
				else {
					x += viewportRect.x;
					y += viewportRect.y;

					lines.push_back(Vec4i(startPoint.x, startPoint.y, x, y));
					line(currentImage, startPoint, Point(x, y), Scalar(0, 0, 255), 2);
					startPoint = Point(x, y);
					updateImage();
				}
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
			if (isDrawing) {
				linesStack.push_back(lines);
			}

			isDrawing = false;
			isDragging = false;
			isZooming = false;
			lockedDir = false;
			lockedX = false;
			lockedY = false;
			lines.clear();

			break;
		case EVENT_MOUSEWHEEL:
			if (flags & EVENT_FLAG_CTRLKEY) {
				isZooming = true;
				if (flags > 0 && zoomLevel < maxZoom) {
					zoomLevel *= 1.1;
					updateImage();
				}
				if (flags < 0 && zoomLevel > -maxZoom) {
					zoomLevel /= 1.1;
					updateImage();
				}
			}
			break;
	}
}

int main()
{
	string imagePath = "C:/Users/Yuri/Desktop/big.jpg";

	originalImage = imread(imagePath, IMREAD_COLOR);	
	resize(originalImage, originalImage, Size(20000, 10000));

	currentImage = originalImage.clone();

	namedWindow("Drawing");

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



