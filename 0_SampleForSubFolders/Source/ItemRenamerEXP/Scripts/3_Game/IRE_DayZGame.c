modded class DayZGame
{
	override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, target, rpc_type, ctx);

		if (rpc_type == EIRE_RPC.IRE_RPC_SYNC_CONFIG)
		{
			Param1<IRE_SyncPacket> data;
			if (!ctx.Read(data) || !data || !data.param1)
			{
				Print("[ItemRenamerEXP] Failed to read sync packet");
				return;
			}

			IRE_Manager.SyncFromPacket(data.param1);
			if (data.param1.Items)
				Print("[ItemRenamerEXP] Client received " + data.param1.Items.Count().ToString() + " item rename(s)");
			return;
		}

		if (rpc_type == EIRE_RPC.IRE_RPC_REQ_RELOAD)
		{
			if (!IsServer())
			{
				return;
			}

			if (!GetIRE_Admin().IsAdmin(sender))
			{
				Print("[ItemRenamerEXP] #irl denied for non-admin guid=" + sender.GetId() + " steam=" + sender.GetPlainId());
				return;
			}

			IRE_ConfigLoader.ReloadAndBroadcast("chat #irl");
		}
	}
};
