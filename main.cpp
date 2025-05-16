//=======================================
// 
// Author	:角 凜太朗
// Date		:5/14
//
//=======================================
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN //古いウィンドウズのファイルを使わないように飛ばすため
#include<Windows.h>

#include <algorithm>

//=======================================
// ウィンドウ情報
//=======================================
static constexpr char WINDOW_CLASS[] = "GameWindow";	//メインウィンドウ名
static constexpr char TITLE[] = "ゲームウィンドウ";		//左上のタイトルバーのテキスト
// constexprはコンパイルをするときに定数化する
// constはビルド時に定数化する

//=======================================
// ウィンドウプロシージャ　プロトタイプ宣言
//=======================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//=======================================
// メイン
//=======================================
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	//ウィンドウクラスの登録
	WNDCLASSEX wcex{};											// 構造体	{}があると無いとでは違う　ない場合はごみが入ってしまう　あると中身を初期化してくれる　WNDCLASSEX wcex = {};でもよい
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;									// ウィンドウプロシージャというものを登録しないといけない(関数ポインタ)
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);			// どんなアイコンにするかを決めている
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);				// カーソルの絵を変えたりする場所
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);			// ウィンドウが出来た時の背景を変えることができる
	wcex.lpszMenuName = nullptr;								// メニューは作らない　本来はいらない上で初期化しているため
	wcex.lpszClassName = WINDOW_CLASS;							// ウィンドウクラス名でWINDOW_CLASSを引き出す
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);	// スモールアイコンを登録できる

	RegisterClassEx(&wcex);										// Registerは登録という意味 wcexの名前で登録


	//クライアント領域のサイズを持った短形
	RECT window_rect = { 0,0,800,450 };
	//ウィンドウのスタイル
	DWORD window_style = WS_EX_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
	//指定したクライアント領域を確保するために新たな短形座標を計算
	AdjustWindowRect(&window_rect, window_style, FALSE);

	//ウィンドウの幅と高さを算出
	int window_width = window_rect.right - window_rect.left;
	int window_height = window_rect.bottom - window_rect.top;

	//プライマリモニターの画面解像度取得
	int desktop_width = GetSystemMetrics(SM_CXSCREEN);
	int desktop_height = GetSystemMetrics(SM_CYSCREEN);

	// デスクトップの真ん中にウィンドウが生成されるように座標を計算
	// ただし万が一、デスクトップよりウィンドウが大きい場合は左上に表示
	int window_x = std::max((desktop_width - window_width) / 2,0);
	int window_y = std::max((desktop_height - window_height) / 2,0);

	//メインウィンドウの作成
	HWND hWnd = CreateWindow(	//hWndは返り値　ハンドル　識別子
		WINDOW_CLASS,
		TITLE,								  	//"あいうえお"でも良い
		//WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,	//ウィンドウスタイルを変えられるフラグ
		window_style,						  	//フラグを管理して、最大化させないようにできる
		window_x,				  				//ウィンドウの初期座標Xを決める　CW_USEDEFAULTは適当な位置に設定してくれる
		window_y,							  	//ウィンドウの初期座標Yを決める
		window_width,						  	//ウィンドウの幅
		window_height,			  			  	//ウィンドウの高さ
		nullptr,	  						  	//親のウィンドウハンドルを作るとき　nullptrは親がいないため入れている
		nullptr,	  						  	//メニュー
		hInstance,
		nullptr
	);

	ShowWindow(hWnd, nCmdShow);	//ウィンドウ表示
	UpdateWindow(hWnd);			//ウィンドウの描画の更新

	//メッセージループ
	//MSG msg;
	//while (GetMessage(&msg, nullptr, 0, 0))//GetMessageはウィンドウからのメッセージを待っている
	//{
	//	TranslateMessage(&msg);			//文字列に変更する
	//	DispatchMessage(&msg);			//ウィンドウプロシージャの中にあるUINT message, WPARAM wParam, LPARAM lParamを読みだす
	//}//戻り値で0が返されるとwhile分から抜けるのでループから終了する
	//return(int)msg.wParam;

	//ゲームループ
	MSG msg;
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))// ウィンドウメッセージが来ていたら
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else// ゲームの処理
		{

		}
	} while (msg.message != WM_QUIT);


}
//=======================================
// ウィンドウプロシージャ
//=======================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:			//ウィンドウの破棄メッセージ　ここを外すとウィンドウを閉じれなくなるため必須
		PostQuitMessage(0);		//WM_QUITメッセージの送信
		break;

	case WM_CLOSE:// ウィンドウが閉じられるときに本当に閉じるかどうかの判別をする
		if (MessageBox(hWnd, "本当に閉じてもよろしいですか？", "確認", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK)
		{
			DestroyWindow(hWnd);//指定のウィンドウにWM_DESTROYメッセージを送る
		}
		break;//DefWindowProc関数にメッセージを流さず終了することによって何もなかったことにする

	case WM_KEYDOWN:// WM_KEYDOWNにはマウスのボタンやキーボードのキーが押されたときに受け取るメッセージ
		if (wParam == VK_ESCAPE)// wParamには押されたキーの仮想キーと呼ばれるキーコードが格納されている
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);// WM_CLOSEメッセージの送信
		}
		break;

	default:
		//通常のメッセージ処理はこの関数に任せる
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//=======================================
// メモ
//=======================================
// 
//スクリーンショットを参照
//Ctrlキーを押しながらクリックで参照できる
//クリックしてF1キーで検索できる
//すべてのメッセージを待っているのでメッセージループでメッセージを待っている
//ウィンドウプロシージャ
// LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// 
// UINT messageにメッセージIDが入る
// オリジナルメッセージIDを作ることも出来る
// 
// プログラムの進行を止めて何かを待つ(同期処理)
// 非同期処理もある
// 
// スワップチェーン
// スワップ、フリップを使うことで、描画を行うバッファと、描画を表示するバッファを分けることができる
// この入れ替え作業のことをスワップ、フリップという
// ティアリングとは画面描画の途中で画面が切り替わることによってズレが生じること
// 
// 




