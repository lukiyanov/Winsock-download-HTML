#pragma once
#include <string>
#include <ostream>

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
		// --------------------------------------------------------------------
		// ��������� html-�������� �� ��������� HTTP (HTTPS �� ��������������)
		// � ������� pageUrl � ������� � � ��������� �����.
		// � ������ ������ ���������� ����������, ����������� �� WinsockException.
		// --------------------------------------------------------------------
		void DownloadPageToDirectory(const std::string& pageUrl, std::ostream& out);

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