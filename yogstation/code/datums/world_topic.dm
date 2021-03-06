/datum/world_topic/asay
	keyword = "asay"
	require_comms_key = TRUE

/datum/world_topic/asay/Run(list/input)
	to_chat(GLOB.admins, "<span class='adminobserver'><span class='prefix'>DISCORD ADMIN:</span> <EM>[input["admin"]]</EM>: <span class='message'>[input["asay"]]</span></span>")

/datum/world_topic/ooc
	keyword = "ooc"
	require_comms_key = TRUE

/datum/world_topic/ooc/Run(list/input)
	for(var/client/C in GLOB.clients)
		to_chat(C, "<font color='[GLOB.normal_ooc_colour]'><span class='ooc'><span class='prefix'>DISCORD OOC:</span> <EM>[input["admin"]]:</EM> <span class='message'>[input["ooc"]]</span></span></font>")

/datum/world_topic/toggleooc
	keyword = "toggleooc"
	require_comms_key = TRUE

/datum/world_topic/toggleooc/Run(list/input)
	toggle_ooc()
	return GLOB.ooc_allowed

/datum/world_topic/adminwho/Run(list/input)
	var/list/message = list("Admins: ")
	var/list/admin_keys = list()
	for(var/adm in GLOB.admins)
		var/client/C = adm
		if(input["adminchannel"])
			admin_keys += "[C][C.holder.fakekey ? "(Stealth)" : ""][C.is_afk() ? "(AFK)" : ""]"
		else if(!C.holder.fakekey)
			admin_keys += "[C]"

	for(var/admin in admin_keys)
		if(LAZYLEN(message) > 1)
			message += ", [admin]"
		else
			message += "[admin]"

	return jointext(message, "")
