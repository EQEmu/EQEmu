/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef EQWX_TASKS__H
#define EQWX_TASKS__H

struct eqtask
{
	unsigned int id;
	unsigned int duration; //i
	char title[100]; //i
	char desc[2047]; //i
	char reward[64]; //i
	unsigned int rewardid; //i
	unsigned int cashreward; //i
	int xpreward; //i
	unsigned short rewardmethod; //i
	unsigned int startzone; //i
	unsigned short level_min; //i
	unsigned short level_max; //i
	bool repeatable;
};

struct eqtask_goallist
{
	unsigned int id;
	unsigned int value;
};

struct eqtask_activities
{
	unsigned int id;
	unsigned int activityId; //i
	unsigned int step; //i
	unsigned int activityType; //i
	char text1[64]; //i
	char text2[64]; //i
	char text3[128]; //i
	unsigned int goalid; //n 7 digit textbox
	unsigned int goalmethod; //i
	unsigned int goalcount; //n 2 digit text box
	unsigned int deliverToNpc; //i
	unsigned int zoneid; //i
	bool optional; //i
};

struct eqtask_activity_id
{
	unsigned int id;
	unsigned int step;
	unsigned int activityid;
};

struct eqtask_zones
{
	char name[32];
	unsigned int id;
};

struct eqtask_proximity
{
	unsigned int exploreid;
	unsigned int zoneid;
	double minx;
	double maxx;
	double miny;
	double maxy;
	double minz;
	double maxz;
};

struct eqtask_prox
{
	unsigned int exploreid;
	unsigned int zoneid;
};

#endif