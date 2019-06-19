#pragma once
#include <string>

namespace task
{
	///////////////////////////////////////////////////////////////////////////
	//
	//	�������� �� ���������� �������� ������ � �������������.
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
		// �������� �� ������������� � ������� ���������� Winsock.
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