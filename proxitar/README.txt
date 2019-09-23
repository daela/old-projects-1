All commands are of the form, bot: <command> [parameters]


ADMIN ONLY COMMANDS
===================
ally_clan [clan list]
deally_clan [clan list]
deally_all_clans

ally_player [player list]
deally_player [player list]
deally_all_players

ignore_clan [clan list]
unignore_clan [clan list]
unignore_all_clans

ignore_player [player list]
unignore_player [player list]
unignore_all_players

ignore_holding [holding list]
unignore_holding [holding list]
unignore_all_holdings

bank auto <NUMBER_OF_SECONDS_UNTIL_AUTO_CLOSE>
^ -- if seconds is 0, it doesn't automatically close

ANYONE COMMANDS
===============
status

seen_player
seen_clan

bank [open|close|0-3]
tower [on|off|0-3]

RESULTED ANYONE COMMANDS
========================
These commands generate a result page.

list - gives a list of the bot settings regarding allies/ignores

unique_allies - gives a list of all the unique allies that have been seen in each of the holdings since reset
current_allies - gives a list of all the allies currently in every holding (like an ally version of 'status')
unique_enemies - gives a list of all the unique enemies that have been seen in each of the holdings since reset
current_enemies - gives a list of all the enemies currently in every holding (just like 'status')

ADMIN ONLY RESULTED COMMANDS
============================
clear_unique - reset the 'unique' player lists


NOTE, READ THIS!!!!!!!!!!!!!
============================
Whenever the bot disconnects/reconnects, the list of unique players is cleared.  This time is stored, and included
in the statistics of the results.  You can manually reset the list (like, prior to a siege!) at any time if you are
an admin.

There is ONLY _ONE_ result page, new result requests overwrite it.  Save what you want to keep.  If the bot recently
lost internet, and then reconnected, it may take up to 30 minutes before you can connect to it via the link. This
will be rare, but is expected.


TO GET SIEGE NUMBERS
====================
pre-siege, use clear_unique, then after the siege, do unique_enemies.... check the list; you got numbers!
