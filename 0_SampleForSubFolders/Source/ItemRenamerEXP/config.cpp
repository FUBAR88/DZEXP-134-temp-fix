class CfgPatches
{
	class ItemRenamerEXP
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};
class CfgMods
{
	class ItemRenamerEXP
	{
		dir = "ItemRenamerEXP";
		name = "ItemRenamerEXP";
		credits = "FUBAR";
		author = "FUBAR";
		authorID = "0";
		version = "1.0";
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"ItemRenamerEXP/Scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"ItemRenamerEXP/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"ItemRenamerEXP/Scripts/5_Mission"};
			};
		};
	};
};
