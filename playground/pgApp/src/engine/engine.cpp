#include "engine.h"
#include <windows.h>

void ReportErrorAndThrow(const std::string& file, int line, const std::string& function, const std::string& message)
{
	std::stringstream ss;

	DWORD errorCode = GetLastError();
	LPTSTR errorMessage = nullptr;

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorMessage, 0, nullptr);

	if (errorMessage)
	{
		ss << file << "(" << line << "): " << "error " << errorCode << ": " << errorMessage << std::endl;
		LocalFree(errorMessage);
	}
	else
	{
		ss << file << "(" << line << "): " << message << std::endl;
	}

	OutputDebugStringA(ss.str().c_str());
	MessageBoxA(nullptr, message.c_str(), function.c_str(), MB_ICONERROR);
	throw new std::exception(message.c_str());
}


pgBasePass::pgBasePass(const pgPassCreateInfo& ci)
	: base(ci)
{
}

pgBasePass::~pgBasePass()
{
}


void pgBasePass::Update(RenderEventArgs& e)
{
}

void pgBasePass::Render(RenderEventArgs& e)
{
}

pgTechnique::pgTechnique()
{}

pgTechnique::~pgTechnique()
{
	m_Passes.clear();
}

unsigned int pgTechnique::addPass(std::shared_ptr<pgPass> pass)
{
	// No check for duplicate passes (it may be intended to render the same pass multiple times?)
	m_Passes.push_back(pass);
	return static_cast<unsigned int>(m_Passes.size()) - 1;
}

std::shared_ptr<pgPass> pgTechnique::getPass(unsigned int ID) const
{
	if (ID < m_Passes.size())
	{
		return m_Passes[ID];
	}

	return 0;
}

void pgTechnique::Update(RenderEventArgs& e)
{
	for (auto pass : m_Passes)
	{
		if (pass->IsEnabled())
		{
			pass->Update(e);
		}
	}
}

// Render the scene using the passes that have been configured.
void pgTechnique::Render(RenderEventArgs& e)
{
	for (auto pass : m_Passes)
	{
		if (pass->IsEnabled())
		{
			pass->Render(e);
		}
	}
}


