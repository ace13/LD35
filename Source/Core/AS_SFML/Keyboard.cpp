#include "Shared.hpp"

#include <SFML/Window/Keyboard.hpp>

namespace
{

}

void as::priv::RegKeyboard(asIScriptEngine* eng)
{
	AS_ASSERT(eng->SetDefaultNamespace("sf::Keyboard"));

	AS_ASSERT(eng->RegisterEnum("Key"));
#define K(Val) AS_ASSERT(eng->RegisterEnumValue("Key", #Val, sf::Keyboard::Val))
	K(A); K(B); K(C); K(D); K(E); K(F); K(G); K(H); K(I); K(J); K(K); K(L); K(M); K(N);
	K(O); K(P); K(Q); K(R); K(S); K(T); K(U); K(V); K(W); K(X); K(Y); K(Z); K(Num0);
	K(Num1); K(Num2); K(Num3); K(Num4); K(Num5); K(Num6); K(Num7); K(Num8); K(Num9);
	K(Escape); K(LControl); K(LShift); K(LAlt); K(LSystem); K(RControl); K(RShift);
	K(RAlt); K(RSystem); K(Menu); K(LBracket); K(RBracket); K(SemiColon); K(Comma);
	K(Period); K(Quote); K(Slash); K(BackSlash); K(Tilde); K(Equal); K(Dash); K(Space);
	K(Return); K(BackSpace); K(Tab); K(PageUp); K(PageDown); K(End); K(Home); K(Insert);
	K(Delete); K(Add); K(Subtract); K(Multiply); K(Divide); K(Left); K(Right); K(Up);
	K(Down); K(Numpad0); K(Numpad1); K(Numpad2); K(Numpad3); K(Numpad4); K(Numpad5);
	K(Numpad6); K(Numpad7); K(Numpad8); K(Numpad9); K(F1); K(F2); K(F3); K(F4); K(F5);
	K(F6); K(F7); K(F8); K(F9); K(F10); K(F11); K(F12); K(F13); K(F14); K(F15); K(Pause);
#undef K

	AS_ASSERT(eng->RegisterGlobalFunction("bool IsPressed(Key)", asFUNCTION(sf::Keyboard::isKeyPressed), asCALL_CDECL));

#if SFML_VERSION_MINOR > 1
	AS_ASSERT(eng->RegisterGlobalFunction("void ShowVirtual(bool)", asFUNCTION(sf::Keyboard::setVirtualKeyboardVisible), asCALL_CDECL));
#endif

	AS_ASSERT(eng->SetDefaultNamespace(""));
}
