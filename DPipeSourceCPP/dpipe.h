#pragma once
#include "dpipebase.h"
#include "dpipea.h"
#include "dpipen.h"
#include "dpstring.h"
#include "dpwstring.h"
#include "dpclient.h"

namespace crdk {
	namespace dpipes {

		class DPipeBuilder {
		public:
			static IDPipe* Create(const DPIPE_TYPE type, DWORD inBufferSize, DWORD outBufferSize);
			static IDPipe* Create(const DPIPE_TYPE type, std::wstring name);
			static IDPipe* Create(const DPIPE_TYPE type, std::wstring name, DWORD inBufferSize, DWORD outBufferSize);
			static IDPipe* Create(const DPIPE_TYPE type);
			static IDPipe* Create(const std::wstring handleString, DWORD inBufferSize, DWORD outBufferSize, bool connect = false);
			static IDPipe* Create(const std::wstring handleString, std::wstring name, DWORD inBufferSize, DWORD outBufferSize, bool connect = false);
			static IDPipe* Create(const std::wstring handleString, const std::wstring name, bool connect = false);
			static IDPipe* Create(const std::wstring handleString, bool connect = false);
		};
	}
}

