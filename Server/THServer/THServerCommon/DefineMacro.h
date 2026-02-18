#pragma once

#define SAFE_DELETE(x)			if (x != nullptr) { delete x; x = nullptr; }
#define SAFE_DELETE_ARRAY(x)	if (x != nullptr) { delete[] x; x = nullptr; }
#define SAFE_FREE(x)			if (x != nullptr) { free(x); x = nullptr; }

#define NEW(cls, ...)			std::make_shared<cls>(__VA_ARGS__)
#define PTR						std::shared_ptr
#define UNIQUE_NEW(cls, ...)	std::make_unique<cls>(__VA_ARGS__)
#define UNIQUE_PTR				std::unique_ptr