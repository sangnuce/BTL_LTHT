#include <Windows.h>
#pragma comment (lib, "WINMM.LIB")

#define ID_TIMER		1
#define ALERT_TIMER		2
#define START_BUTTON	101
#define STOP_BUTTON		102
#define HOUR_BOX		103
#define MINUTE_BOX		104

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("Alarm");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	HBRUSH hBrush = CreateSolidBrush(RGB(127, 219, 255));
	wndclass.hbrBackground = hBrush;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("Program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName, TEXT("Ứng dụng hẹn giờ đơn giản"),
		WS_OVERLAPPEDWINDOW,
		400, 100,
		500, 300,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeleteObject(hBrush);
	return msg.wParam;
}

// Hàm kiểm tra 1 chuỗi có phải là các chữ số
BOOL isDigit(wchar_t num[])
{
	for (int i = 0; i < lstrlen(num); i++) {
		if (num[i]<'0' || num[i]>'9') return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int	hour = 0;
	static int	minute = 0;
	static BOOL	running = FALSE;
	SYSTEMTIME st;
	HWND		hWndEdit;
	wchar_t		buffer[256];
	HDC			hdc;
	PAINTSTRUCT	ps;

	switch (message)
	{
	case WM_CREATE:
		// Tạo ô TEXTBOX để nhập giờ
		CreateWindow(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER, 50, 70, 50, 24, hwnd, (HMENU)HOUR_BOX, GetModuleHandle(NULL), NULL);

		// Tạo ô TEXTBOX để nhập phút
		CreateWindow(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER, 110, 70, 50, 24, hwnd, (HMENU)MINUTE_BOX, GetModuleHandle(NULL), NULL);

		// Tạo nút bấm để bắt đầu đếm
		CreateWindow(L"BUTTON", L"BẮT ĐẦU", WS_VISIBLE | WS_CHILD, 50, 100, 100, 24, hwnd, (HMENU)START_BUTTON, GetModuleHandle(NULL), NULL);

		// Tạo 1 nút bấm để ngừng báo
		CreateWindow(L"BUTTON", L"DỪNG", WS_VISIBLE | WS_CHILD, 160, 100, 100, 24, hwnd, (HMENU)STOP_BUTTON, GetModuleHandle(NULL), NULL);
		return 0;

	case WM_TIMER:
		switch (wParam)
		{
		case ID_TIMER:

			// Lấy thời gian hiện tại của hệ thống
			GetLocalTime(&st);

			//Vẽ lại vùng client để cập nhật hiển thị thời gian còn lại
			InvalidateRect(hwnd, NULL, TRUE);

			// Kiểm tra xem đã đến thời điểm cảnh báo hay chưa
			if (hour == st.wHour && minute == st.wMinute && st.wSecond == 0)
			{
				//Hiện cửa sổ lên
				ShowWindow(hwnd, SW_SHOW);
				UpdateWindow(hwnd);

				//Đánh dấu báo thức ngừng chạy
				running = FALSE;

				// Đặt bộ đếm thời gian để phát âm báo
				SetTimer(hwnd, ALERT_TIMER, 1000, NULL);

				// Huỷ bộ đếm thời gian kiểm tra giờ
				KillTimer(hwnd, ID_TIMER);

				//Hiển thị thông báo
				MessageBoxW(hwnd, TEXT("Đã đến thời điểm hẹn giờ!"), TEXT("Nhắc nhở"), MB_OK);

				// Huỷ bộ đếm thời gian phát âm báo
				KillTimer(hwnd, ALERT_TIMER);
			}
			break;
		case ALERT_TIMER:

			// Phát âm thanh cảnh báo
			Beep(4000, 100);
			Beep(4000, 100);
			Beep(4000, 100);
			break;
		}
		return 0;

	case WM_COMMAND: // Kiểm tra xem có nút nào được bấm hay không
		switch (LOWORD(wParam))
		{
		case START_BUTTON: // Bấm vào nút BẮT ĐẦU
			// Kiểm tra xem có báo thức đang chạy không
			if (running == TRUE)
			{
				// Nếu có thì cảnh báo
				MessageBoxW(hwnd, TEXT("Báo thức đang chạy! Hãy dừng báo thức trước khi cài lại báo thức!"), TEXT("Cảnh báo"), MB_OK | MB_ICONWARNING);

				// Thoát khỏi phần xử lý thông điệp
				return 0;
			}
			// Biến kiểm tra xem dữ liệu nhập vào có phải là số hay không
			BOOL checkHour, checkMinute;

			//Lấy phần tử có ID là HOUR_BOX nằm trong cửa sổ hwnd
			hWndEdit = GetDlgItem(hwnd, HOUR_BOX);
			// Lấy thời gian trong ô HOUR_BOX gán vào biến buffer
			GetWindowText(hWndEdit, buffer, 256);
			// Chuyển đổi buffer sang số nguyên và gán cho giờ báo thức.
			hour = _wtoi(buffer);

			// kiểm tra giờ nhập vào có phải là số
			checkHour = isDigit(buffer);


			//Lấy phần tử có ID là MINUTE_BOX nằm trong cửa sổ hwnd
			hWndEdit = GetDlgItem(hwnd, MINUTE_BOX);
			// Lấy thời gian trong ô MINUTE_BOX gán vào biến buffer
			GetWindowText(hWndEdit, buffer, 256);
			// Chuyển đổi buffer sang số nguyên và gán cho phút báo thức.
			minute = _wtoi(buffer);

			// kiểm tra phút nhập vào có phải là số
			checkMinute = isDigit(buffer);

			// Kiểm tra xem thời gian nhaaj pvafo có hợp lệ không
			if (hour < 0 || hour>23 || minute < 0 || minute>59 || checkHour == FALSE || checkMinute == FALSE)
			{
				// Nếu không thì cảnh báo
				MessageBoxW(hwnd, TEXT("Thời gian không hợp lệ. (Phải trong khoảng 00:00 đến 23:59)"), TEXT("Cảnh báo"), MB_OK | MB_ICONWARNING);

				// Thoát khỏi phần xử lý thông điệp
				return 0;
			}

			//Đánh dấu chạy báo thức
			running = TRUE;
				
			// Khởi tạo bộ đếm thời gian
			SetTimer(hwnd, ID_TIMER, 1000, NULL);

			// Vẽ lại vùng Client
			InvalidateRect(hwnd, NULL, TRUE);

			break;

		case STOP_BUTTON: // Bấm vào nút STOP
			
			//Đánh dấu báo thức ngừng chạy
			running = FALSE;

			// Huỷ bộ đếm thời gian
			KillTimer(hwnd, ID_TIMER);

			// Vẽ lại vùng Client
			InvalidateRect(hwnd, NULL, TRUE);

			break;
		}
		return 0;

	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);

		SetBkMode(hdc, TRANSPARENT);

		// Hiển thị lời nhắc nhập thời gian
		wsprintf(buffer, TEXT("Nhập vào thời gian hẹn giờ:"));
		TextOutW(hdc, 50, 30, buffer, lstrlen(buffer));

		// Hiển thị lời nhắc nhập thời gian
		wsprintf(buffer, TEXT("Giờ"));
		TextOutW(hdc, 60, 50, buffer, lstrlen(buffer));
		wsprintf(buffer, TEXT(":"));
		TextOutW(hdc, 103, 70, buffer, lstrlen(buffer));
		wsprintf(buffer, TEXT("Phút"));
		TextOutW(hdc, 125, 50, buffer, lstrlen(buffer));

		// Nếu dang cài báo thức thì hiển thị giờ báo
		if (running == TRUE)
		{
			// Hiển thị chuỗi báo thời gian còn lại
			wsprintf(buffer, TEXT("Đã đặt báo thức vào %d : %d"), hour, minute);
			TextOutW(hdc, 50, 130, buffer, lstrlen(buffer));
		}
		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_CLOSE:
		if (running == TRUE)
		{
			//Nếu đang chạy báo thức mà click đóng cửa sổ thì hỏi xác nhận.
			int option = MessageBoxW(hwnd, TEXT("Báo thức đang chạy, bạn có muốn ứng dụng chạy ngầm?"), TEXT("Cảnh báo"), MB_YESNOCANCEL | MB_ICONWARNING);
			if (option == IDYES)
			{
				//Ẩn cửa sổ đi
				ShowWindow(hwnd, SW_HIDE);
				return 0;
			}
			else if (option == IDCANCEL) return 0; //Cancel thì không làm gì cả
			//Còn nếu chọn No thì chạy xuống case WM_DESTROY giống như khi báo thức không chạy.
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}