#ifndef CRAFTING_UTILS_H
# define CRAFTING_UTILS_H

# include <vector>
# include "SKSE_resolver.h"
# include RESOLVE_PATH(SKSE_FOLDER, GameEvents.h)

struct StaticFunctionTag;
class EnchantmentItem;
class GFxMovieView;
class GFxValue;
class VMClassRegistry;
class InventoryEntryData;

namespace CraftingUtils
{
	struct						t_item
	{
		struct					details
		{
			EnchantmentItem		*enchantment;
			float				charge;
			float				temperval;
			bool isnew;

			explicit			details(void);
								details(const details&);
								details& operator = (const details&) = delete;
								//~details(void);
		};
		typedef std::vector<details> t_stats;
		t_stats					stats;
		SInt32					instances;
		UInt32					created;

		explicit				t_item(void);
		explicit				t_item(t_stats*);
								t_item(const t_item&) = delete;
								t_item& operator = (const t_item&) = delete;
								//~t_item_data(void);
	};

	static_assert(std::is_standard_layout<t_item::details>::value == true, "Error: expected standard layout.");

	struct						t_service
	{
		enum					category {craft = 1, enchant, temper};
		category				type;
		bool					listening;
	};

	struct						menuEventHandler : public BSTEventSink <MenuOpenCloseEvent>
	{
		virtual EventResult		ReceiveEvent(MenuOpenCloseEvent *evn, EventDispatcher<MenuOpenCloseEvent> *dsp) override;
	};

	void						onUIRefresh(GFxMovieView *view, GFxValue *object, InventoryEntryData *item);
	bool						registerPapyrusFunctions(VMClassRegistry *registry);
	
}
#endif //CraftingUtils.h
