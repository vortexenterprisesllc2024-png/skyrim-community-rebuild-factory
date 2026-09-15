#include "CraftingUtils.h"
#include "IniParser.h"
#include "common/IDebugLog.h"
#include RESOLVE_PATH(SKSE_FOLDER, GameMenus.h)
#include RESOLVE_PATH(SKSE_FOLDER, GameExtraData.h)
#include RESOLVE_PATH(SKSE_FOLDER, GameRTTI.h)
#include RESOLVE_PATH(SKSE_FOLDER, PapyrusNativeFunctions.h)
#include RESOLVE_PATH(SKSE_FOLDER, PapyrusActor.h)
#include RESOLVE_PATH(SKSE_FOLDER, GameData.h)
#include <map>


namespace CraftingUtils
{
	t_item::details::details(void)
	{	memset(this, 0, sizeof(*this) );
		temperval = 1.f;
	}
	t_item::details::details(const details &d)
	{	memcpy(this, &d, sizeof(*this) );
	}
	t_item::t_item(void)
	{	instances = 0; created = 0;
	}
	t_item::t_item(t_stats *newstats) : t_item()
	{	const size_t size = (*newstats).size();

		if (size)
		{	stats.resize(size);
			memcpy(stats.data(), (*newstats).data(), sizeof(*stats.data() ) * size);
		}
	}

	static std::map<TESForm*, t_item>	records;
	static menuEventHandler				menusink;
	static t_service					service;
	static bool							menu_open;


	EventResult menuEventHandler::ReceiveEvent(MenuOpenCloseEvent *evn, EventDispatcher<MenuOpenCloseEvent> *disptchr)
	{
		if ((*evn).menuName.data == (*UIStringHolder::GetSingleton() ).craftingMenu.data)
			menu_open = (*evn).opening;
		return (kEvent_Continue);
		(void)disptchr;
	}


	static InventoryEntryData *get_pcinvdata_for_item(TESForm *item)
	{
		ExtraContainerChanges	*changes = (ExtraContainerChanges*)((**g_thePlayer).extraData).GetByType(kExtraData_ContainerChanges);
		EntryDataList::Iterator	invdata;

		if (changes && (*changes).data && (*(*changes).data).objList)
		{	invdata = (*(*(*changes).data).objList).Begin();
			while (!invdata.End() && (**invdata).type != item)
				++invdata;
		}
		return (invdata.Get() );
	}


	static bool find_stat(const t_item::t_stats *list, const t_item::details *stat, const bool findnew = false)
	{
		constexpr size_t		section = offsetof(t_item::details, t_item::details::isnew);
		const t_item::details	*element = (*list).data();

		for (unsigned i = (*list).size(); i; --i)
		{	if (memcmp(element, stat, section) || (findnew && !(*element).isnew) )
			{	++element;
				continue ;
			}
			return (true);
		}
		return (false);
	}


	static SInt32 get_pcinventory_count(TESForm *item)
	{
		ExtraContainerChanges	*changes = (ExtraContainerChanges*)((**g_thePlayer).extraData).GetByType(kExtraData_ContainerChanges);
		TESContainer			*basecont = DYNAMIC_CAST((**g_thePlayer).baseForm, TESForm, TESContainer);
		EntryDataList::Iterator	entry;
		SInt32					count = 0;

		if (basecont)
			count = (*basecont).CountItem(item);
		if (changes && (*changes).data && (*(*changes).data).objList)
		{	entry = (*(*(*changes).data).objList).Begin();
			while (!entry.End() && (**entry).type != item)
				++entry;
			if (*entry)
				count += (**entry).countDelta;
		}
		return (count & -!(count & 0x80000000) );
	}


	static bool get_item_stats(t_item::t_stats *stats, const InventoryEntryData *entry, const SInt32 count)
	{
		SInt32 added = 0;

		if (entry && (*entry).extendDataList)
		{	for (ExtendDataList::Iterator itemdata = (*(*entry).extendDataList).Begin(); itemdata.Get(); ++itemdata)
			{	TESEnchantableForm	*basenchant;
				ExtraEnchantment	*xtrenchant;
				ExtraHealth			*itemhealth;
				t_item::details		details;

				if ((itemhealth = (ExtraHealth*)((BaseExtraList&)(**itemdata) ).GetByType(kExtraData_Health) ) )
					details.temperval = (*itemhealth).health;
				if ((xtrenchant = (ExtraEnchantment*)((BaseExtraList&)(**itemdata) ).GetByType(kExtraData_Enchantment) ) )
					details.enchantment = (*xtrenchant).enchant;
				else if ((basenchant = DYNAMIC_CAST((*entry).type, TESForm, TESEnchantableForm) ) )
					details.enchantment = (*basenchant).enchantment;
				if (details.enchantment && (*(*entry).type).formType == kFormType_Weapon)
				{	ExtraCharge *itemcharge = (ExtraCharge*)((BaseExtraList&)(**itemdata) ).GetByType(kExtraData_Charge);

					if (itemcharge && (*itemcharge).charge)
						details.charge = (*itemcharge).charge;
					else if (xtrenchant)
						details.charge = (float)(int)(*xtrenchant).maxCharge;
					else if (basenchant)
						details.charge = (float)(int)(*basenchant).maxCharge;
				}
				(*stats).push_back(details);
				++added;
			}
		}
		if (added < count)
			(*stats).emplace_back();
		return (added);
	}


	static void find_moded(InventoryEntryData *entry, t_item::t_stats *stats)
	{
		std::map<TESForm*, t_item>::iterator pos = records.find((*entry).type);

		get_item_stats(stats, entry, (*entry).countDelta);
		if (menu_open)
		{	for (t_item::t_stats::iterator it = (*stats).begin(), offbound = (*stats).end(); it != offbound; ++it)
			{	if (!find_stat(&(*pos).second.stats, &*it))
				{	(*it).isnew = true;
					(*pos).second.stats.emplace_back(*it);
					++(*pos).second.created;
				}
			}
			return ;
		}
		if (pos == records.end() )
			(*(records.emplace((*entry).type, stats).first) ).second.instances = get_pcinventory_count((*entry).type);
		else for (t_item::t_stats::iterator it = (*stats).begin(), offbound = (*stats).end(); it != offbound; ++it)
		{	if (!find_stat(&(*pos).second.stats, &*it) )
				(*pos).second.stats.emplace_back(*it);
		}
	}


	static void find_crafted(const SInt32 count, InventoryEntryData *entry, t_item::t_stats *stats)
	{
		std::map<TESForm*, t_item>::iterator pos = records.find((*entry).type);
		InventoryEntryData *invdata = (invdata = get_pcinvdata_for_item((*entry).type) ) ? invdata : entry;

		if (menu_open)
		{	bool isnew = false;

			get_item_stats(stats, invdata, count);
			if (pos == records.end() )
				pos = (records.emplace((*entry).type, stats).first);
			else for (t_item::t_stats::iterator stat = (*stats).begin(); stat != (*stats).end(); ++stat)
			{	if (!find_stat(&(*pos).second.stats, &*stat))
				{	(*pos).second.stats.emplace_back(*stat);
					isnew = true;
				}
			}
			if (count > (*pos).second.instances)
			{	(*pos).second.created += (count - (*pos).second.instances);
				(*pos).second.instances = count;
				isnew = true;
			}
			if (isnew)
				(*pos).second.stats.back().isnew = true;
		}
		else if (pos == records.end() )
		{	get_item_stats(stats, invdata, count);
			(*(records.emplace((*entry).type, stats).first)).second.instances = count;
		}
	}


	extern void onUIRefresh(GFxMovieView *view, GFxValue *object, InventoryEntryData *data)
	{
		if (!service.listening)
			return;
		static t_item::t_stats	stats;
		SInt32					count;

		switch (service.type)
		{	case t_service::enchant:
				if ((*(*data).type).formType == kFormType_Enchantment || (*(*data).type).formType == kFormType_SoulGem)
					return ;
			case t_service::temper:
				find_moded(data, &stats);
			break;
			case t_service::craft:
				if (count = get_pcinventory_count((*data).type) )
					find_crafted(count, data, &stats);
			break ;
			default:
			break;
		}
		stats.clear();
		(void)view, (void)object;
	}


	extern bool registerForCraftingSession(StaticFunctionTag*, UInt32 session_type)
	{
		switch (session_type)
		{	case t_service::craft:	
			case t_service::enchant:
			case t_service::temper:
				(*(*MenuManager::GetSingleton() ).MenuOpenCloseEventDispatcher() ).AddEventSink(&menusink);
			break;
			case 0:
				(*(*MenuManager::GetSingleton() ).MenuOpenCloseEventDispatcher() ).RemoveEventSink(&menusink);
				records.clear();
			break;
			default:
				return (false);
		}
		service.type = (t_service::category)session_type;
		service.listening = !!session_type;
		return (true);
	}


	/* Honed Metal 1.26.1 / HonedMetal.bsa HMCraftingUtils:
	 *   Int[] Function getItemsCount(ObjectReference container, Form[] items) native global
	 * Published Nexus "Source Code" v7 registered getCachedItemCount(REFR*, Form*) instead.
	 * Extra-change deltas are cached per container (same idea as v7); TESContainer base
	 * counts are added per requested form so NPC stock items are visible. */
	extern VMResultArray<SInt32> getItemsCount(StaticFunctionTag*, TESObjectREFR *src, VMArray<TESForm*> items)
	{
		static std::map<TESForm*, SInt32>	extra_delta;
		static TESObjectREFR				*last = nullptr;
		VMResultArray<SInt32>				result;
		const UInt32						n = items.Length();
		TESContainer						*container = nullptr;

		result.resize(n);
		if (!src)
		{	extra_delta.clear();
			last = nullptr;
			for (UInt32 i = 0; i < n; ++i)
				result[i] = -1;
			return (result);
		}
		if (last != src)
		{	last = src;
			extra_delta.clear();
			ExtraContainerChanges *inventory = (ExtraContainerChanges*)(*src).extraData.GetByType(kExtraData_ContainerChanges);
			EntryDataList		  *itemlst;

			if (inventory && (*inventory).data && (itemlst = (*(*inventory).data).objList))
			{	for (tList<InventoryEntryData>::Iterator it = (*itemlst).Begin(); !it.End(); ++it)
					extra_delta[(**it).type] = (**it).countDelta;
			}
		}
		container = DYNAMIC_CAST((*src).baseForm, TESForm, TESContainer);
		for (UInt32 i = 0; i < n; ++i)
		{	TESForm									*form = nullptr;
			SInt32									count = 0;
			std::map<TESForm*, SInt32>::const_iterator	extra;

			items.Get(&form, i);
			if (!form)
			{	result[i] = -1;
				continue ;
			}
			if (container)
				count += (SInt32)(*container).CountItem(form);
			extra = extra_delta.find(form);
			if (extra != extra_delta.end() )
				count += (*extra).second;
			else if (!container)
			{	result[i] = -1;
				continue ;
			}
			result[i] = count;
		}
		return (result);
	}


	extern UInt32 getDisplayCostForItem(StaticFunctionTag*, TESObjectREFR *container, TESForm *item)
	{
		UInt32 cost = 0;

		if (container && item)
		{	ExtraContainerChanges *inventory = (ExtraContainerChanges*)(*container).extraData.GetByType(kExtraData_ContainerChanges);
			InventoryEntryData *invdata;

			if (inventory && (invdata = (*(*inventory).data).FindItemEntry(item)))
				cost = CALL_MEMBER_FN(invdata, GetValue)();
		}
		return (cost);
	}


	extern UInt32 getSkillReqForPerk(StaticFunctionTag*, BGSPerk *perk, BSFixedString actorvalue)
	{
		if (perk && (*perk).conditions)
		{	constexpr unsigned	GREATER_OR_EQUAL = 0X9fU;
			constexpr UInt16	GET_BASE_AV = 0X115;
			Condition			*current = (*perk).conditions;
			const UInt32		id = ActorValueList::ResolveActorValueByName(actorvalue.data);

			while (current)
			{	if ((*current).functionId != GET_BASE_AV ||
				(((*current).comparisonType) & GREATER_OR_EQUAL) || (id && (*current).param1 != id) )
				{	current = (*current).next;
					continue;
				}
				return ((UInt32)(int)*(float*)&(*current).compareValue);
			}
		}
		return (0);
	}


	extern VMResultArray<BGSPerk*> getPerksFromIni(StaticFunctionTag*, UInt32 type)
	{
		VMResultArray<BGSPerk*>	blacklisted;
		BGSPerk					**perks = GET_PERK_ENTRIES(DataHandler::GetSingleton() );
		const char				*section = type ? "DUALENCH" : "BLACKLIST";
		constexpr UInt32		BITMASK = 0x00ffffffU;
		std::vector<UInt32>		blacklist;
		
		if (iniparser::parse(&blacklist, section) )
		{	for (unsigned i = GET_PERKS_COUNT(DataHandler::GetSingleton() ); i; --i)
			{	for (size_t j = blacklist.size() - 1; j < INT_MAX; --j)
				{	if (((**perks).formID & BITMASK) != blacklist[j])
						continue;
					blacklisted.push_back(*perks);
					_MESSAGE("Perk: %x has been verified.", (**perks).formID);
					break;
				}
				++perks;
			}
		}
		_MESSAGE("Search for %s returned: %u perk(s).", (const char *)section, blacklisted.size() );
		return (blacklisted);
	}


	extern bool addBaseItems(StaticFunctionTag*,TESObjectREFR *container, VMArray<TESForm*> forms, VMArray<SInt32> counts, bool remove = false)
	{
		ExtraContainerChanges *dst = (ExtraContainerChanges*)(*container).extraData.GetByType(kExtraData_ContainerChanges);

		if (dst && (*dst).data && (*(*dst).data).objList && forms.Length() == counts.Length() )
		{	InventoryEntryData	*entry;
			TESForm				*item;
			SInt32				count;

			for (UInt32 idx = forms.Length() -1; idx < INT_MAX; --idx)
			{	forms.Get(&item, idx);
				counts.Get(&count, idx);
				if (!(item && count) )
					continue ;
				tList<InventoryEntryData>::Iterator lst = (*(*(*dst).data).objList).Begin();

				while ((entry = lst.Get()) && (*entry).type != item)
					++lst;
				switch ((!!entry << 1) | remove)
				{   case 0b00:
						entry = (InventoryEntryData*)Heap_Allocate(sizeof(InventoryEntryData) );
						new (entry) InventoryEntryData(item, count);
						(*(*(*dst).data).objList).Insert(entry);
					break ;
					case 0b01:
					break ;
					case 0b10:
					case 0b11:
						if (((*entry).countDelta += (count ^ -remove) + remove) < 0)
							(*entry).countDelta = 0;
					break ;
				}
			}
			return (true);
		}
		return (false);
	}


	static inline InventoryEntryData *get_entry_data(ExtraContainerChanges *cont, TESForm *item)
	{
		InventoryEntryData *entry = (*(*cont).data).FindItemEntry(item);

		if (!entry)
		{	entry = InventoryEntryData::Create(item, 0);
			(*(*(*cont).data).objList).Insert(entry);
		}
		return (entry);
	}


	static inline void unequip_worn(TESObjectREFR *owner, InventoryEntryData *entry)
	{
		const SInt32	count = (*(*entry).type).IsAmmo() ? (*entry).countDelta : 1;
		BaseExtraList	*wornleft = nullptr;
		BaseExtraList	*worn = nullptr;

		(*entry).GetExtraWornBaseLists(&worn, &wornleft);
		if (worn)
			CALL_MEMBER_FN(EquipManager::GetSingleton(), UnequipItem)((Actor*)owner, (*entry).type, worn, count, GetRightHandSlot(), false, false, true, false, NULL);
		if (wornleft)
			CALL_MEMBER_FN(EquipManager::GetSingleton(), UnequipItem)((Actor*)owner, (*entry).type, wornleft, count, GetLeftHandSlot(), false, false, true, false, NULL);
	}


	extern bool transferProducedItems(StaticFunctionTag*, TESObjectREFR *dst, TESObjectREFR *src)
	{
		if (!(src && dst) )
			return (false);
		bool transfer_success = false;
		ExtraContainerChanges *dstcont = (ExtraContainerChanges*)(*dst).extraData.GetByType(kExtraData_ContainerChanges);

		if (dstcont && (*dstcont).data)
		{	ExtraContainerChanges *srcont = (ExtraContainerChanges*)(*src).extraData.GetByType(kExtraData_ContainerChanges);
			std::map<TESForm*, t_item>::const_iterator entries = records.cbegin();
			InventoryEntryData *entry;

			for (std::map<TESForm*, t_item>::const_iterator cend = records.cend(); entries != cend; ++entries)
			{	if (!((*entries).second.created && (entry = get_entry_data(srcont, (*entries).first) ) ) )
					continue ;
				t_item::t_stats entry_stats;
				const t_item *item = &(*entries).second;
				t_item::t_stats::const_iterator stat, end;
				ExtendDataList::Iterator srcdata = (*(*entry).extendDataList).Begin();
				SInt32 pos = 0;

				get_item_stats(&entry_stats, entry, (*item).instances + (*item).created);
				for (stat = entry_stats.cbegin(), end = entry_stats.cend(); stat != end; ++stat)
				{	if (find_stat(&(*item).stats, &*stat, true) )
					{	tList<InventoryEntryData>::Iterator dstentries = (*(*(*dstcont).data).objList).Begin();
						InventoryEntryData *dstentry;

						unequip_worn(src, entry);
						while ((dstentry = dstentries.Get() ) && (*dstentry).type != (*entry).type)
							++dstentries;
						if (!dstentry)
						{	dstentry = InventoryEntryData::Create((*entry).type, (*item).created);
							(*(*(*dstcont).data).objList).Insert(dstentry);
						}
						else (*dstentry).countDelta += (*item).created;
						(*entry).countDelta -= (*item).created;
						if (srcdata.Get() && (*(**srcdata).m_data).GetType() != kExtraData_Count)
						{	(*(*dstentry).extendDataList).Insert(srcdata.Get() );
							for (SInt32 idx = pos; *srcdata; ++idx)
								(*(*entry).extendDataList).ReplaceNth(idx, (++srcdata).Get() );
						}
						transfer_success = true;
						break ;
					}
					++srcdata, ++pos;
				}
			}
		}
		return (transfer_success);
	}


	extern float rechargeWeapon(StaticFunctionTag*, TESObjectREFR *container, TESObjectWEAP *weapon, const bool all = false)
	{
		if (container && weapon)
		{	ExtraContainerChanges	*changes = (ExtraContainerChanges*)(*container).extraData.GetByType(kExtraData_ContainerChanges);
			InventoryEntryData		*entry = nullptr;
			TESFullName				*formname = DYNAMIC_CAST(weapon, TESForm, TESFullName);
			const char				*name = formname ? (*formname).GetName() : nullptr;

			if (!(changes && (*changes).data) )
				return (false);
			if ((entry = (*(*changes).data).FindItemEntry(weapon) ) )
			{	ExtendDataList	*xtradata_list = (*entry).extendDataList;
				BaseExtraList	*xdata = nullptr;
				float			recharged = 0.f;

				for (ExtendDataList::Iterator xlist = (*xtradata_list).Begin(); xdata = xlist.Get(); ++xlist)
				{	ExtraCharge *charge = (ExtraCharge *)(*xdata).GetByType(kExtraData_Charge);
					
					if (charge)
					{	ExtraEnchantment *xenchant;
						const float initial = (*charge).charge;

						if ((*weapon).enchantable.enchantment)
							(*charge).charge = (float)(int)(*weapon).enchantable.maxCharge;
						else if ((xenchant = (ExtraEnchantment*)(*xdata).GetByType(kExtraData_Enchantment) ) )
							(*charge).charge = (float)(int)(*xenchant).maxCharge;
						recharged += (*charge).charge - initial;
						if (recharged && !all)
							break ;
					}
				}
				return (recharged);
			}
		}
		return (0.f);
	}


	extern bool registerPapyrusFunctions(VMClassRegistry *registry)
	{
		#define thread_safe_flag VMClassRegistry::kFunctionFlag_NoWait
		(*registry).RegisterFunction(new NativeFunction1<StaticFunctionTag, bool, UInt32>(
			"registerForCraftingSession", "HMCraftingUtils", registerForCraftingSession, registry) );
		(*registry).RegisterFunction(new NativeFunction1<StaticFunctionTag, VMResultArray<BGSPerk*>, UInt32>(
			"getPerksFromIni", "HMCraftingUtils", getPerksFromIni, registry) );
		(*registry).RegisterFunction(new NativeFunction2<StaticFunctionTag, UInt32, TESObjectREFR*, TESForm*>(
			"getDisplayCostForItem", "HMCraftingUtils", getDisplayCostForItem, registry) );
		(*registry).RegisterFunction(new NativeFunction2<StaticFunctionTag, VMResultArray<SInt32>, TESObjectREFR*, VMArray<TESForm*>>(
			"getItemsCount", "HMCraftingUtils", getItemsCount, registry) );
		(*registry).RegisterFunction(new NativeFunction2<StaticFunctionTag, UInt32, BGSPerk*, BSFixedString>(
			"getSkillReqForPerk", "HMCraftingUtils", getSkillReqForPerk, registry) );
		(*registry).RegisterFunction(new NativeFunction2<StaticFunctionTag, bool, TESObjectREFR*, TESObjectREFR*>(
			"transferProducedItems", "HMCraftingUtils", transferProducedItems, registry));
		(*registry).RegisterFunction(new NativeFunction3<StaticFunctionTag, float, TESObjectREFR *, TESObjectWEAP *, bool>(
			"rechargeWeapon", "HMCraftingUtils", rechargeWeapon, registry));
		(*registry).RegisterFunction(new NativeFunction4<StaticFunctionTag, bool, TESObjectREFR *, VMArray<TESForm*>, VMArray<SInt32>, bool>(
			"addBaseItems", "HMCraftingUtils", addBaseItems, registry));
		(*registry).SetFunctionFlags("HMCraftingUtils", "registerForCraftingSession", thread_safe_flag);
		(*registry).SetFunctionFlags("HMCraftingUtils", "getPerksFromIni", thread_safe_flag);
		(*registry).SetFunctionFlags("HMCraftingUtils", "getDisplayCostForItem", thread_safe_flag);
		(*registry).SetFunctionFlags("HMCraftingUtils", "getItemsCount", thread_safe_flag);
		(*registry).SetFunctionFlags("HMCraftingUtils", "getSkillReqForPerk", thread_safe_flag);
		_MESSAGE("Papyrus registrations complete.");
		return (true);
	}
}