#include "dpipe.h"
#include <regex>

using namespace std;
using namespace crdk::dpipes;

//DPipeBuilder implementation
#pragma region DPipeBuilder
	IDPipe* DPipeBuilder::Create(const DPIPE_TYPE type, DWORD inBufferSize, DWORD outBufferSize) {
		switch (type) {
		case DPIPE_TYPE::ANONYMOUS_PIPE:
			return new DPipeAnonymous(inBufferSize, outBufferSize);

		case DPIPE_TYPE::NAMED_PIPE:
			return new DPipeNamed(inBufferSize, outBufferSize);
		default:
			return nullptr;
		}
	}

	IDPipe* DPipeBuilder::Create(const DPIPE_TYPE type, const std::wstring name)
	{
		switch(type) {
		case DPIPE_TYPE::ANONYMOUS_PIPE:
			return new DPipeAnonymous(name);

		case DPIPE_TYPE::NAMED_PIPE:
			return DPipeNamed::Create(name);
		default:
			return nullptr;
		}
	}

	IDPipe* DPipeBuilder::Create(const DPIPE_TYPE type, std::wstring name, DWORD inBufferSize, DWORD outBufferSize)
	{
		switch (type) {
		case DPIPE_TYPE::ANONYMOUS_PIPE:
			return new DPipeAnonymous(name, inBufferSize, outBufferSize);

		case DPIPE_TYPE::NAMED_PIPE:
			return DPipeNamed::Create(name, inBufferSize, outBufferSize);
		default:
			return nullptr;
		}
	}

	IDPipe* DPipeBuilder::Create(const DPIPE_TYPE type)
	{
		switch (type) {
		case DPIPE_TYPE::ANONYMOUS_PIPE:
			return new DPipeAnonymous();

		case DPIPE_TYPE::NAMED_PIPE:
			return new DPipeNamed();
		default:
			return nullptr;
		}
	}

	IDPipe* DPipeBuilder::Create(const std::wstring pipeHandleString, DWORD inBufferSize, DWORD outBufferSize, bool connect)
	{
		if (DPipeAnonymousHandle::IsAnonymous(pipeHandleString))
		{
			auto anonymousPipe = new DPipeAnonymous(inBufferSize, outBufferSize);
			if (connect)
				anonymousPipe->Connect(pipeHandleString);
			return anonymousPipe;
		}

		else if (DPipeNamedHandle::IsNamed(pipeHandleString))
		{
			auto namedPipe = new DPipeNamed(inBufferSize, outBufferSize);
			if (connect)
				namedPipe->Connect(pipeHandleString);
			return namedPipe;
		}
		else
			return nullptr;
	}

	IDPipe* DPipeBuilder::Create(const std::wstring pipeHandleString, const std::wstring name, DWORD inBufferSize, DWORD outBufferSize, bool connect)
	{
		if (DPipeAnonymousHandle::IsAnonymous(pipeHandleString))
		{
			auto anonymousPipe = new DPipeAnonymous(name, inBufferSize, outBufferSize);
			if (connect)
				anonymousPipe->Connect(pipeHandleString);
			return anonymousPipe;
		}

		else if (DPipeNamedHandle::IsNamed(pipeHandleString))
		{
			auto namedPipe = DPipeNamed::Create(name, inBufferSize, outBufferSize);
			if (connect)
				namedPipe->Connect(pipeHandleString);
			return namedPipe;
		}
		else
			return nullptr;
	}

	IDPipe* DPipeBuilder::Create(const std::wstring pipeHandleString, const std::wstring name, bool connect)
	{
		if (DPipeAnonymousHandle::IsAnonymous(pipeHandleString))
		{
			auto anonymousPipe = new DPipeAnonymous(name);
			if (connect)
				anonymousPipe->Connect(pipeHandleString);
			return anonymousPipe;
		}

		else if (DPipeNamedHandle::IsNamed(pipeHandleString))
		{
			auto namedPipe = DPipeNamed::Create(name);
			if (connect)
				namedPipe->Connect(pipeHandleString);
			return namedPipe;
		}
		else
			return nullptr;
	}

	IDPipe* DPipeBuilder::Create(const std::wstring pipeHandleString, bool connect)
	{
		if (DPipeAnonymousHandle::IsAnonymous(pipeHandleString))
		{
			auto anonymousPipe = new DPipeAnonymous();
			if (connect)
				anonymousPipe->Connect(pipeHandleString);
			return anonymousPipe;
		}

		else if (DPipeNamedHandle::IsNamed(pipeHandleString))
		{
			auto namedPipe = new DPipeNamed();
			if (connect)
				namedPipe->Connect(pipeHandleString);
			return namedPipe;
		}
		else
			return nullptr;
	}
#pragma endregion DPipeBuilder