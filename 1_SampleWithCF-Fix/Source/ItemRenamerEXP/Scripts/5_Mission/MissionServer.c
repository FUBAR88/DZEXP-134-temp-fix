modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		if (!g_Game || !g_Game.IsServer())
		{
			return;
		}

		IRE_ConfigLoader.Load();
		Print("[ItemRenamerEXP] Server ready (ConfigVersion=" + IRE_ConfigLoader.GetConfig().ConfigVersion.ToString() + "). Item cache in 5 seconds");
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.BuildItemCache, IRE_Constants.STARTUP_CACHE_DELAY_MS, false);
	}

	void BuildItemCache()
	{
		int count;

		if (!g_Game || !g_Game.IsServer())
		{
			return;
		}

		IRE_ConfigLoader.RefreshItemCache();
		IRE_Manager.SyncFromEntries(IRE_ConfigLoader.GetEntries());
		IRE_ConfigLoader.BroadcastConfig();
		count = 0;
		if (IRE_ConfigLoader.GetEntries())
		{
			count = IRE_ConfigLoader.GetEntries().Count();
		}
		Print("[ItemRenamerEXP] Startup item cache built - " + count.ToString() + " item(s)");
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);
		IRE_ConfigLoader.SendConfigToPlayer(player);
	}
};
