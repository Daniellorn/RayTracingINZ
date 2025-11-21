#pragma once

#include <chrono>

namespace App {

	class Timer
	{
	public:

		Timer()
		{
			Reset();
		}

		void Reset()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		float ElapsedTime()
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(
				std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f;
		}

	private:

		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	};
}