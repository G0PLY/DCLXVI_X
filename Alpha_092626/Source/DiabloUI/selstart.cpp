#include "selstart.h"

#include "DiabloUI/diabloui.h"
#include "control.h"
#include "engine/load_clx.hpp"
#include "options.h"
#include "utils/language.h"
#include "utils/sdl_geometry.h"

namespace devilution {
namespace {

bool endMenu;

std::vector<std::unique_ptr<UiListItem>> vecDialogItems;
std::vector<std::unique_ptr<UiItemBase>> vecDialog;

void ItemSelected(int value)
{
	auto option = static_cast<StartUpGameMode>(vecDialogItems[value]->m_value);
	sgOptions.StartUp.gameMode.SetValue(option);
	SaveOptions();
	endMenu = true;
}

void EscPressed()
{
	endMenu = true;
}

} // namespace

void UiSelStartUpGameOption()
{
	// Reset palette/fade/backbuffer state before drawing this menu.
	UiLoadBlackBackground();
	ArtBackgroundWidescreen = LoadOptionalClx("ui_art\\mainmenuw.clx");
	ArtBackground = std::nullopt;
	UiAddBackground(&vecDialog);
	// The DCLXVI background already contains its own title treatment.
	// UiAddLogo(&vecDialog);

	const Point uiPosition = GetUIRectangle().position;
	vecDialogItems.push_back(std::make_unique<UiListItem>(_("Enter Hellfire"), static_cast<int>(StartUpGameMode::Hellfire)));
	vecDialogItems.push_back(std::make_unique<UiListItem>(_("Switch to Diablo"), static_cast<int>(StartUpGameMode::Diablo)));
	vecDialog.push_back(std::make_unique<UiList>(vecDialogItems, vecDialogItems.size(), uiPosition.x + 64, uiPosition.y + 240, 510, 43, UiFlags::AlignCenter | UiFlags::FontSize42 | UiFlags::ColorUiSilver, 5));

	UiInitList(nullptr, ItemSelected, EscPressed, vecDialog, true);

	endMenu = false;
	while (!endMenu) {
		UiClearScreen();
		UiRenderItems(vecDialog);
		UiPollAndRender();
	}

	// Destroy UI items before releasing the CLX storage they reference.
	vecDialogItems.clear();
	vecDialog.clear();
	ArtBackground = std::nullopt;
	ArtBackgroundWidescreen = std::nullopt;

	// Fully reset palette, fade state, and the UI backbuffer before returning.
	// A plain UiLoadDefaultPalette() does not reset UiFadeIn's fade state.
	UiLoadBlackBackground();
}

} // namespace devilution
