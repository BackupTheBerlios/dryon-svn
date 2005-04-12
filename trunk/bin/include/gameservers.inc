#pragma library GameServers

native hl_ping(const host[], port= 27015);
native hl_readinfos_int(hl_infos:id, const what[]);
native hl_readinfos_string(hl_infos:id, const what[], dest[], dest_maxsize= sizeof dest);
native hl_infos:hl_servinfos(const host[], port= 27015);
native hl_rcon_close(hl_rcon:id);
native hl_rcon_cmd(hl_rcon:id, const cmd[], out[], out_maxsize= sizeof out);
native hl_rcon:hl_rcon_init(const pass[], const host[], port= 27015);
