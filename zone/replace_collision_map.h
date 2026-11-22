#pragma once

#include <memory>

// Zone owns current. Call only between queries on its exclusive zone thread.
// Taking the candidate by value keeps its ownership scoped through installation.
template <typename T>
bool ReplaceCollisionMap(T *&current, std::unique_ptr<T> candidate) noexcept
{
	if (!candidate) {
		return false;
	}
	auto previous = std::unique_ptr<T>(current);
	current = candidate.release();
	return true;
}
