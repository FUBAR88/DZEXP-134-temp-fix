modded class ChatInputMenu
{
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		string text;
		string lower;

		if (!finished)
			return false;

		text = m_edit_box.GetText();
		text.TrimInPlace();
		lower = text;
		lower.ToLower();

		if (lower == IRE_Constants.CHAT_RELOAD)
		{
			if (g_Game && g_Game.IsMultiplayer())
			{
				g_Game.RPCSingleParam(null, EIRE_RPC.IRE_RPC_REQ_RELOAD, new Param1<bool>(true), true, null);
			}
			else if (g_Game && g_Game.IsServer())
			{
				IRE_ConfigLoader.ReloadAndBroadcast("offline #irl");
			}

			m_close_timer.Run(0.1, this, "Close");
			GetUApi().GetInputByID(UAPersonView).Supress();
			return true;
		}

		return super.OnChange(w, x, y, finished);
	}
};
