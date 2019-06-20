#pragma once
#include <string>
#include <ostream>

namespace task
{
	///////////////////////////////////////////////////////////////////////////
	//
	//	Отвечает за скачивание страницы вместе с зависимостями.
	//
	///////////////////////////////////////////////////////////////////////////
	class PageDownloader
	{
	public:
		// --------------------------------------------------------------------
		// Скачивает html-страницу по протоколу HTTP (HTTPS не поддерживается)
		// с адресом pageUrl и выводит её в указанный поток.
		// В случае ошибок генерирует исключения, производные от WinsockException.
		// --------------------------------------------------------------------
		void DownloadPageToDirectory(const std::string& pageUrl, std::ostream& out);

	private:

		// --------------------------------------------------------------------
		// Отвечает за инициализацию и очистку библиотеки Winsock.
		// --------------------------------------------------------------------
		class WinsockInitializerAndCleaner
		{
		public:
			WinsockInitializerAndCleaner();
			~WinsockInitializerAndCleaner();
		private:
			bool IsWinsockInitialized();
			void InitializeWinsock();
			void CleanupWinsock();
		};

		static WinsockInitializerAndCleaner m_winsockInitializerAndCleaner;
		// --------------------------------------------------------------------
	};

	///////////////////////////////////////////////////////////////////////////
}