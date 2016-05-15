#include <Windows.h>
#pragma comment (lib, "WINMM.LIB")

#define ID_TIMER		1
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
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("Program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName, TEXT("Ứng dụng hẹn giờ đơn giản"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int	hour = 0;
	static int	minute = 0;
	static int	hours_left = 0;
	static int	minutes_left = 0;
	static int	seconds_left = 0;
	static int	running = 0;
	SYSTEMTIME st;
	HWND		hWndEdit;
	wchar_t		buffer[256];
	HDC			hdc;
	PAINTSTRUCT	ps;

	switch (message)
	{
	case WM_CREATE:
		// Tạo ô TEXTBOX để nhập giờ
		CreateWindow(L"EDIT",
			L"0",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			50,
			70,
			50,
			24,
			hwnd,
			(HMENU)HOUR_BOX,
			GetModuleHandle(NULL),
			NULL);

		// Tạo ô TEXTBOX để nhập phút
		CreateWindow(L"EDIT",
			L"0",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			110,
			70,
			50,
			24,
			hwnd,
			(HMENU)MINUTE_BOX,
			GetModuleHandle(NULL),
			NULL);

		// Tạo nút bấm để bắt đầu đếm
		CreateWindow(L"BUTTON",
			L"BẮT ĐẦU",
			WS_VISIBLE | WS_CHILD,
			50,
			100,
			100,
			24,
			hwnd,
			(HMENU)START_BUTTON,
			GetModuleHandle(NULL),
			NULL);

		// Tạo 1 nút bấm để ngừng báo
		CreateWindow(L"BUTTON",
			L"DỪNG",
			WS_VISIBLE | WS_CHILD,
			160,
			100,
			100,
			24,
			hwnd,
			(HMENU)STOP_BUTTON,
			GetModuleHandle(NULL),
			NULL);
		return 0;

	case WM_TIMER:
		GetLocalTime(&st);

		//Tính thời gian còn lại
		if (st.wHour > hour)
		{
			hours_left = 24 - st.wHour + hour;
		}
		else
		{
			hours_left = hour - st.wHour;
		}

		if (st.wMinute > minute)
		{
			minutes_left = 60 - st.wMinute + minute;
			if (hours_left == 0) hours_left = 23;
			else hours_left--;
		}
		else
		{
			minutes_left = minute - st.wMinute;
		}

		if (st.wSecond > 0)
		{
			seconds_left = 60 - st.wSecond;
			if (minutes_left == 0) minutes_left = 59;
			else minutes_left--;
			if (st.wHour == hour && st.wMinute == minute) hours_left = 23;
		}
		else
		{
			seconds_left = 0;
		}

		//Vẽ lại vùng client để cập nhật hiển thị thời gian còn lại
		InvalidateRect(hwnd, NULL, TRUE);

		// Kiểm tra xem đã đến thời điểm cảnh báo hay chưa
		if (hours_left == 0 && minutes_left == 0 && seconds_left <= 0)
		{
			// Huỷ bộ đếm thời gian
			KillTimer(hwnd, ID_TIMER);
			
			//Hiện cửa sổ lên
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);

			//Đánh dấu báo thức ngừng chạy
			running = 0;

			// Phát âm thanh cảnh báo
			PlaySound(TEXT("pager_tone.wav"), NULL, SND_FILENAME | SND_ASYNC);

			//Hiển thị thông báo
			MessageBoxW(hwnd, TEXT("Đã đến giờ hoàng đạo rồi bạn thân yêu ơi!"), TEXT("Chúc mừng năm mới!!! ^_^"), MB_OK);
			
			//Dừng âm thanh cảnh báo khi đóng hộp thoại thông báo
			PlaySound(NULL, 0, 0);
		}
		break;

	case WM_COMMAND: // Kiểm tra xem có nút nào được bấm hay không
		switch (LOWORD(wParam))
		{
		case START_BUTTON: // Bấm vào nút BẮT ĐẦU
			if (running == 0)
			{
				//Lấy phần tử có ID là HOUR_BOX nằm trong cửa sổ hwnd
				hWndEdit = GetDlgItem(hwnd, HOUR_BOX);
				// Lấy thời gian trong ô HOUR_BOX gán vào biến buffer
				GetWindowText(hWndEdit, buffer, 256);
				// Chuyển đổi buffer sang số nguyên và gán cho giờ báo thức.
				hour = _wtoi(buffer);

				//Lấy phần tử có ID là MINUTE_BOX nằm trong cửa sổ hwnd
				hWndEdit = GetDlgItem(hwnd, MINUTE_BOX);
				// Lấy thời gian trong ô MINUTE_BOX gán vào biến buffer
				GetWindowText(hWndEdit, buffer, 256);
				// Chuyển đổi buffer sang số nguyên và gán cho phút báo thức.
				minute = _wtoi(buffer);

				if (hour < 0 || hour>23 || minute < 0 || minute>59)
				{
					MessageBoxW(hwnd, TEXT("Thời gian không hợp lệ. (Phải trong khoảng 00:00 đến 23:59)"), TEXT("Cảnh báo"), MB_OK | MB_ICONWARNING);
				}
				else
				{
					//Đánh dấu chạy báo thức
					running = 1;
					// Khởi tạo bộ đếm thời gian
					SetTimer(hwnd, ID_TIMER, 1000, NULL);
				}
			}
			else
			{
				MessageBoxW(hwnd, TEXT("Báo thức đang chạy! Hãy dừng báo thức trước khi cài lại báo thức!"), TEXT("Cảnh báo"), MB_OK | MB_ICONWARNING);
			}
			break;

		case STOP_BUTTON: // Bấm vào nút STOP
			// Ngừng âm thanh cảnh báo.
			PlaySound(NULL, 0, 0);
			//Đánh dấu báo thức ngừng chạy
			running = 0;
			// Huỷ bộ đếm thời gian
			KillTimer(hwnd, ID_TIMER);
			break;
		}
		break;

	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);

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

		// Hiển thị chuỗi báo thời gian còn lại
		wsprintf(buffer, TEXT("Cảnh báo trong %d giờ %d phút %d giây!"), hours_left, minutes_left, seconds_left);
		TextOutW(hdc, 50, 130, buffer, lstrlen(buffer));

		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_CLOSE:
		if (running == 1)
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