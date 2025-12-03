#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <algorithm>  // std::max, std::min用

namespace hal
{
    /**
     * @brief デバッグコンソール: ゲームとは別ウィンドウでリアルタイムデバッグログを表示
     * 
     * 使用例:
     *   DebugConsole::GetInstance().Log("test message");
     *   DebugConsole::GetInstance().LogFormat("HP: %d / %d", hp, maxHp);
     */
    class DebugConsole
    {
    public:
        static DebugConsole& GetInstance()
        {
            static DebugConsole instance;
            return instance;
        }

        /**
         * @brief コンソールウィンドウを初期化（ゲーム起動時に呼び出す）
         * @param enableConsole true でコンソール表示、false で非表示
         */
        void Initialize(bool enableConsole = true)
        {
            if (m_initialized) return;
            m_initialized = true;
            m_enabled = enableConsole;

            if (!m_enabled) return;

            // 既存のコンソール割り当てを確認
            if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())
            {
                // 標準出力をコンソールにリダイレクト
                FILE* pFile = nullptr;
                freopen_s(&pFile, "CONOUT$", "w", stdout);
                freopen_s(&pFile, "CONOUT$", "w", stderr);
                freopen_s(&pFile, "CONIN$", "r", stdin);

                // C++ の cout/cerr もコンソールに同期
                std::ios::sync_with_stdio(true);
                std::cout.clear();
                std::cerr.clear();

                // ウィンドウタイトル設定
                SetConsoleTitleA("Game Debug Console");

                // コンソール出力バッファをクリア
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                if (hConsole != INVALID_HANDLE_VALUE)
                {
                    DWORD dwWritten;
                    COORD coord = { 0, 0 };
                    DWORD dwConSize;

                    GetConsoleScreenBufferInfo(hConsole, &csbi);
                    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
                    FillConsoleOutputCharacter(hConsole, TEXT(' '), dwConSize, coord, &dwWritten);
                }

                m_consoleReady = true;
            }
        }

        /**
         * @brief シンプルなメッセージをログ出力
         */
        void Log(const char* message)
        {
            if (!m_enabled) return;
            
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_consoleReady)
                {
                    std::cout << message << std::endl;
                    std::cout.flush();
                }
            }
        }

        /**
         * @brief フォーマット付きログ出力（printf風）
         */
        void LogFormat(const char* format, ...)
        {
            if (!m_enabled) return;

            char buffer[1024];
            va_list args;
            va_start(args, format);
            vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
            va_end(args);

            Log(buffer);
        }

        /**
         * @brief 警告メッセージ（黄色で表示）
         */
        void Warning(const char* message)
        {
            if (!m_enabled) return;

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            LogFormat("[WARNING] %s", message);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }

        /**
         * @brief エラーメッセージ（赤で表示）
         */
        void Error(const char* message)
        {
            if (!m_enabled) return;

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
            LogFormat("[ERROR] %s", message);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }

        /**
         * @brief 成功メッセージ（緑で表示）
         */
        void Success(const char* message)
        {
            if (!m_enabled) return;

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            LogFormat("[SUCCESS] %s", message);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }

        /**
         * @brief コンソールをクリア
         */
        void Clear()
        {
            if (!m_enabled || !m_consoleReady) return;

            system("cls");
        }

        /**
         * @brief カーソルを先頭に戻す（上書き表示モード）
         */
        void ResetCursor()
        {
            if (!m_enabled || !m_consoleReady) return;

            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole != INVALID_HANDLE_VALUE)
            {
                COORD coord = { 0, 0 };
                SetConsoleCursorPosition(hConsole, coord);
            }
        }

        /**
         * @brief コンソール画面をクリアしてカーソルをリセット
         */
        void ClearScreen()
        {
            Clear();
            ResizeConsoleWindow();
        }

        /**
         * @brief コンソールウィンドウサイズを自動調整（内容に合わせて縮小）
         */
        void ResizeConsoleWindow()
        {
            if (!m_enabled || !m_consoleReady) return;

            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole == INVALID_HANDLE_VALUE) return;

            // 現在のコンソール情報を取得
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;

            // 実際に使用されている領域を計算
            // テキストの最後の行を検索
            COORD cursorPos = csbi.dwCursorPosition;
            
            // 最小サイズと最大サイズを設定
            int minWidth = 80;   // 最小幅
            int maxWidth = 200;  // 最大幅
            int minHeight = 50;  // 最小高さ
            int maxHeight = 80;  // 最大高さ

            // カーソル位置に基づいて、必要なサイズを計算
            int requiredWidth = cursorPos.X + 10;  // 余裕を持たせる
            int requiredHeight = cursorPos.Y + 3;  // 余裕を持たせる

            // 範囲内に制限
            int newWidth = (std::max)(minWidth, (std::min)(maxWidth, requiredWidth));
            int newHeight = (std::max)(minHeight, (std::min)(maxHeight, requiredHeight));

            // ウィンドウサイズを設定（コンソールバッファサイズの変更）
            // ウィンドウサイズはバッファサイズに制限される
            SMALL_RECT windowSize = {
                0,
                0,
                static_cast<SHORT>(newWidth - 1),
                static_cast<SHORT>(newHeight - 1)
            };

            SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
        }

        /**
         * @brief コンソール有効状態を取得
         */
        bool IsEnabled() const { return m_enabled && m_consoleReady; }

        /**
         * @brief コンソールウィンドウを閉じる（終了時）
         */
        void Shutdown()
        {
            if (m_consoleReady)
            {
                FreeConsole();
                m_consoleReady = false;
            }
        }

    private:
        DebugConsole() : m_initialized(false), m_enabled(false), m_consoleReady(false) {}
        ~DebugConsole() { Shutdown(); }

        // コピーコンストラクタと代入演算子を削除
        DebugConsole(const DebugConsole&) = delete;
        DebugConsole& operator=(const DebugConsole&) = delete;

        bool m_initialized;
        bool m_enabled;
        bool m_consoleReady;
        std::mutex m_mutex;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
    };

    // グローバルなデバッグコンソール参照用マクロ
#define DEBUG_LOG(msg) hal::DebugConsole::GetInstance().Log(msg)
#define DEBUG_LOGF(fmt, ...) hal::DebugConsole::GetInstance().LogFormat(fmt, __VA_ARGS__)
#define DEBUG_WARN(msg) hal::DebugConsole::GetInstance().Warning(msg)
#define DEBUG_WARNF(fmt, ...) hal::DebugConsole::GetInstance().LogFormat("[WARNING] " fmt, __VA_ARGS__)
#define DEBUG_ERROR(msg) hal::DebugConsole::GetInstance().Error(msg)
#define DEBUG_ERRORF(fmt, ...) hal::DebugConsole::GetInstance().LogFormat("[ERROR] " fmt, __VA_ARGS__)
#define DEBUG_SUCCESS(msg) hal::DebugConsole::GetInstance().Success(msg)
#define DEBUG_SUCCESSF(fmt, ...) hal::DebugConsole::GetInstance().LogFormat("[SUCCESS] " fmt, __VA_ARGS__)
}
