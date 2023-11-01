#pragma once

#include <cstdint>

#include <SDL.h>

#include "controls/plrctrls.h"
#include "controls/touch/gamepad.h"
#include "engine/surface.hpp"
#include "utils/png.h"
#include "utils/sdl_ptrs.h"
#include "utils/stdcompat/optional.hpp"

namespace devilution {

enum VirtualGamepadButtonType : uint8_t {
	GAMEPAD_ATTACK,
	GAMEPAD_ATTACKDOWN,
	GAMEPAD_TALK,
	GAMEPAD_TALKDOWN,
	GAMEPAD_ITEM,
	GAMEPAD_ITEMDOWN,
	GAMEPAD_OBJECT,
	GAMEPAD_OBJECTDOWN,
	GAMEPAD_CASTSPELL,
	GAMEPAD_CASTSPELLDOWN,
	GAMEPAD_BACK,
	GAMEPAD_BACKDOWN,
	GAMEPAD_BLANK,
	GAMEPAD_BLANKDOWN,
	GAMEPAD_APPLY,
	GAMEPAD_APPLYDOWN,
	GAMEPAD_EQUIP,
	GAMEPAD_EQUIPDOWN,
	GAMEPAD_DROP,
	GAMEPAD_DROPDOWN,
	GAMEPAD_STAIRS,
	GAMEPAD_STAIRSDOWN,
	GAMEPAD_STAND,
	GAMEPAD_STANDDOWN,
	GAMEPAD_POTION,
	GAMEPAD_POTIONDOWN,
};

enum VirtualGamepadPotionType : uint8_t {
	GAMEPAD_HEALING,
	GAMEPAD_MANA,
	GAMEPAD_REJUVENATION,
	GAMEPAD_FULL_HEALING,
	GAMEPAD_FULL_MANA,
	GAMEPAD_FULL_REJUVENATION,
	GAMEPAD_ARENA_POTION,
	GAMEPAD_SCROLL_OF_HEALING,
};

struct ButtonTexture {
	SDLSurfaceUniquePtr surface;
	SDLTextureUniquePtr texture;
	unsigned numSprites = 1;
	unsigned numFrames = 1;

	Size size() const;

	void clear()
	{
		surface = nullptr;
		texture = nullptr;
		numFrames = 1;
	}
};

typedef std::function<void(const ButtonTexture &art, SDL_Rect *src, SDL_Rect *dst)> RenderFunction;

class VirtualMenuPanelRenderer {
public:
	VirtualMenuPanelRenderer(VirtualMenuPanel *virtualMenuPanel)
	    : virtualMenuPanel(virtualMenuPanel)
	{
	}

	void LoadArt(SDL_Renderer *renderer);
	void Render(RenderFunction renderFunction);
	void UnloadArt();

private:
	VirtualMenuPanel *virtualMenuPanel;
	ButtonTexture menuArt;
	ButtonTexture menuArtLevelUp;
};

class VirtualDirectionPadRenderer {
public:
	VirtualDirectionPadRenderer(VirtualDirectionPad *virtualDirectionPad)
	    : virtualDirectionPad(virtualDirectionPad)
	{
	}

	void LoadArt(SDL_Renderer *renderer);
	void Render(RenderFunction renderFunction);
	void UnloadArt();

private:
	VirtualDirectionPad *virtualDirectionPad;
	ButtonTexture padArt;
	ButtonTexture knobArt;

	void RenderPad(RenderFunction renderFunction);
	void RenderKnob(RenderFunction renderFunction);
};

class VirtualPadButtonRenderer {
public:
	VirtualPadButtonRenderer(VirtualPadButton *virtualPadButton)
	    : virtualPadButton(virtualPadButton)
	{
	}

	void Render(RenderFunction renderFunction, const ButtonTexture &buttonArt);

protected:
	VirtualPadButton *virtualPadButton;

	virtual VirtualGamepadButtonType GetButtonType() = 0;
};

class StandButtonRenderer : public VirtualPadButtonRenderer {
public:
	StandButtonRenderer(VirtualPadButton *standButton)
	    : VirtualPadButtonRenderer(standButton)
	{
	}

private:
	VirtualGamepadButtonType GetButtonType();
};

class PrimaryActionButtonRenderer : public VirtualPadButtonRenderer {
public:
	PrimaryActionButtonRenderer(VirtualPadButton *primaryActionButton)
	    : VirtualPadButtonRenderer(primaryActionButton)
	{
	}

private:
	VirtualGamepadButtonType GetButtonType();
	VirtualGamepadButtonType GetTownButtonType();
	VirtualGamepadButtonType GetDungeonButtonType();
	VirtualGamepadButtonType GetInventoryButtonType();
};

class SecondaryActionButtonRenderer : public VirtualPadButtonRenderer {
public:
	SecondaryActionButtonRenderer(VirtualPadButton *secondaryActionButton)
	    : VirtualPadButtonRenderer(secondaryActionButton)
	{
	}

private:
	VirtualGamepadButtonType GetButtonType();
};

class SpellActionButtonRenderer : public VirtualPadButtonRenderer {
public:
	SpellActionButtonRenderer(VirtualPadButton *spellActionButton)
	    : VirtualPadButtonRenderer(spellActionButton)
	{
	}

private:
	VirtualGamepadButtonType GetButtonType();
};

class CancelButtonRenderer : public VirtualPadButtonRenderer {
public:
	CancelButtonRenderer(VirtualPadButton *cancelButton)
	    : VirtualPadButtonRenderer(cancelButton)
	{
	}

private:
	VirtualGamepadButtonType GetButtonType();
};

class PotionButtonRenderer : public VirtualPadButtonRenderer {
public:
	PotionButtonRenderer(VirtualPadButton *potionButton, belt_item_type potionType)
	    : VirtualPadButtonRenderer(potionButton)
	    , potionType(potionType)
	{
	}

	void RenderPotion(RenderFunction renderFunction, const ButtonTexture &potionArt);

private:
	belt_item_type potionType;

	VirtualGamepadButtonType GetButtonType();
	std::optional<VirtualGamepadPotionType> GetPotionType();
};

class VirtualGamepadRenderer {
public:
	VirtualGamepadRenderer(VirtualGamepad *virtualGamepad)
	    : menuPanelRenderer(&virtualGamepad->menuPanel)
	    , directionPadRenderer(&virtualGamepad->directionPad)
	    , standButtonRenderer(&virtualGamepad->standButton)
	    , primaryActionButtonRenderer(&virtualGamepad->primaryActionButton)
	    , secondaryActionButtonRenderer(&virtualGamepad->secondaryActionButton)
	    , spellActionButtonRenderer(&virtualGamepad->spellActionButton)
	    , cancelButtonRenderer(&virtualGamepad->cancelButton)
	    , healthButtonRenderer(&virtualGamepad->healthButton, BLT_HEALING)
	    , manaButtonRenderer(&virtualGamepad->manaButton, BLT_MANA)
	{
	}

	void LoadArt(SDL_Renderer *renderer);
	void Render(RenderFunction renderFunction);
	void UnloadArt();

private:
	VirtualMenuPanelRenderer menuPanelRenderer;
	VirtualDirectionPadRenderer directionPadRenderer;
	StandButtonRenderer standButtonRenderer;

	PrimaryActionButtonRenderer primaryActionButtonRenderer;
	SecondaryActionButtonRenderer secondaryActionButtonRenderer;
	SpellActionButtonRenderer spellActionButtonRenderer;
	CancelButtonRenderer cancelButtonRenderer;

	PotionButtonRenderer healthButtonRenderer;
	PotionButtonRenderer manaButtonRenderer;

	ButtonTexture buttonArt;
	ButtonTexture potionArt;
};

void InitVirtualGamepadGFX(SDL_Renderer *renderer);
void RenderVirtualGamepad(SDL_Renderer *renderer);
void RenderVirtualGamepad(SDL_Surface *surface);
void FreeVirtualGamepadGFX();

} // namespace devilution
