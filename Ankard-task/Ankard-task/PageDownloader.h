#pragma once
#include <string>

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
		// ...
		//PageDownloader();

		// ...
		void DownloadPageToDirectory(const std::string& pageUrl, const std::string& directory);

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