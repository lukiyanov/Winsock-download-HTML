#pragma once
#include <string_view>
#include <list>
#include "TagRecognizer.h"

namespace task
{
	///////////////////////////////////////////////////////////////////////////
	//
	//	�������� �� ���������� �������� ������ � �������������.
	//	� ������ ������ ���������� ����������, ����������� �� WinsockException.
	//	�� ���� �������� url ������ ���� ������ ���������, ������� ��������, ��������:
	//		http://123.45.67.8/some/page.html
	//		http://qwerty.com/another/page/
	//		http://qwerty.com/logo.jpg
	//		http://qwerty.com/
	//
	///////////////////////////////////////////////////////////////////////////
	class HttpDownloader
	{
	public:
		// --------------------------------------------------------------------
		// ��������� �������������� ����.
		// --------------------------------------------------------------------
		void AddDependencyRecognizer(const TagRecognizer& tagRecognizer);

		// --------------------------------------------------------------------
		// ��������� ���� �� ���������� URL �� ��������� HTTP � ���������� ���
		// ���������� � ���� ������.
		// --------------------------------------------------------------------
		std::string DownloadFile(std::string_view pageUrl);

		// --------------------------------------------------------------------
		// ��������� �� ���������� URL HTML-�������� �� ��������� HTTP � ���������
		// � ��� difectory/fileName.html
		// ����� � ����� difectory/fileName ����� ������� ��������� �������.
		// --------------------------------------------------------------------
		void DownloadPageWithDependencies(std::string_view difectory, std::string_view fileName, std::string_view pageUrl);

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

	private:
		std::list<TagRecognizer> m_recognizers;
	};

	///////////////////////////////////////////////////////////////////////////
}