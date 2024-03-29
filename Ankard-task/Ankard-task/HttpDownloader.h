#pragma once
#include <string_view>
#include <list>
#include <functional>
#include "TagRecognizer.h"

namespace task
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	�������� �� ���������� �������� ������ � �������������.
	//	� ������ ������ ���������� ����������, ����������� �� WinsockException.
	//	�� ���� �������� url ������ ���� ������ ���������, ������� ��������, ��������:
	//		http://123.45.67.8/some/page.html
	//		http://qwerty.com/another/page/
	//		http://qwerty.com/logo.jpg
	//		http://qwerty.com/
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class HttpDownloader
	{
	public:
		// ------------------------------------------------------------------------------------------------------------
		// ��������� �������������� ����.
		// ------------------------------------------------------------------------------------------------------------
		void AddDependencyRecognizer(const TagRecognizer& tagRecognizer);

		// ------------------------------------------------------------------------------------------------------------
		// ��������� ���� �� ���������� URL �� ��������� HTTP � ������� ���
		// ���������� � ��������� �����.
		// ------------------------------------------------------------------------------------------------------------
		void DownloadFile(std::string_view pageUrl, std::ostream& out);

		// ------------------------------------------------------------------------------------------------------------
		// ��������� �� ���������� URL (pageUrl) HTML-�������� �� ��������� HTTP � ���������
		// � ��� difectory/fileName.html
		// ����� � ����� difectory/fileName ����� ������� ��������� �������.
		// ���� �����-�� �� ������������ ������� �� �������, ���������� failedDependencyDownloadProcessor
		// � URI � ���������, ������� �� ������� �������.
		// ------------------------------------------------------------------------------------------------------------
		void DownloadPageWithDependencies(
			std::string_view difectory,
			std::string_view fileName,
			std::string_view pageUrl,
			std::function<void(const std::string&, const std::exception&)> failedDependencyDownloadProcessor
		);


	private:

		// ------------------------------------------------------------------------------------------------------------
		// �������� �� ������������� � ������� ���������� Winsock.
		// ------------------------------------------------------------------------------------------------------------
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
		// ------------------------------------------------------------------------------------------------------------

	private:
		std::list<TagRecognizer> m_recognizers;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}