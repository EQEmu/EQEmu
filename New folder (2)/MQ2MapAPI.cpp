#include "MQ2Map.h"

#define pMap     ((PEQMAPWINDOW)pMapViewWnd)
typedef struct _MAPSPAWN
{
    PSPAWNINFO pSpawn;
    eSpawnType SpawnType;

    PMAPLABEL pMapLabel;
    PMAPLINE pVector;
    BOOL Highlight;
    BOOL Explicit;

    struct _MAPSPAWN *pLast;
    struct _MAPSPAWN *pNext;
} MAPSPAWN, *PMAPSPAWN;

#define FAKESPAWNTYPE 0xFF


PMAPSPAWN pActiveSpawns=0;

PMAPLABEL pLabelList=0;
PMAPLABEL pLabelListTail=0;
PMAPLINE  pLineList=0;
PMAPLINE  pLineListTail=0;


void GenerateSpawnLabel(PMAPSPAWN& pMapSpawn, DWORD Color, DWORD TrackingDist);
map<unsigned long,PMAPSPAWN> SpawnMap;
map<unsigned long,PMAPSPAWN> GroundItemMap;
map<PMAPLABEL,PMAPSPAWN> LabelMap;

BOOL Update=false;

#define CASTRADIUS_ANGLESIZE 10
PMAPLINE pCastRadius[(360/CASTRADIUS_ANGLESIZE)+1];

PMAPLINE pSpellRadius[(360/CASTRADIUS_ANGLESIZE)+1];

PMAPSPAWN pLastTarget=0;

PMAPLINE pTargetRadius[(360/CASTRADIUS_ANGLESIZE)+1];

PMAPLINE pTargetMelee[(360/CASTRADIUS_ANGLESIZE)+1];

PMAPLINE pTargetLine=0;


inline PMAPLINE InitLine()
{
    PMAPLINE pLine=new MAPLINE;
    pLine->pPrev=0;
    pLine->pNext=pLineList;
    if (pLineList)
        pLineList->pPrev=pLine;
    else
        pLineListTail=pLine;
    pLineList=pLine;
    return pLine;
}

inline void DeleteLine(PMAPLINE pLine)
{
    if (pLine->pNext)
        pLine->pNext->pPrev=pLine->pPrev;
    else
        pLineListTail=pLine->pPrev;
    if (pLine->pPrev)
        pLine->pPrev->pNext=pLine->pNext;
    else
        pLineList=pLine->pNext;
    delete pLine;
}

inline PMAPLABEL InitLabel(PMAPSPAWN pMapSpawn)
{
    PMAPLABEL pLabel=new MAPLABEL;
    pLabel->pPrev=0;
    pLabel->pNext=pLabelList;
    if (pLabelList)
        pLabelList->pPrev=pLabel;
    else
        pLabelListTail=pLabel;
    pLabelList=pLabel;
    LabelMap[pLabel]=pMapSpawn;
    return pLabel;
}

DWORD GetTrackingAmount()
{
    DWORD trackingDist = 100;
    
    if (PCHARINFO2 pCharInfo = GetCharInfo2())
    {
        if (pLocalPlayer->Data.pSpawn->GM)
            trackingDist = 999999;
        trackingDist += 10 * pCharInfo->Skill[53];
    }
    /*
    if (((PCHARINFO)pCharData) && ((PCHARINFO)pCharData)->pCI2)
	{
        /*
		for (unsigned long nGem = 0; nGem < NUM_SPELL_GEMS; nGem++)
		{
			if (PCHARINFO2 pCharInfo = GetCharInfo2())
			{

                auto trackingDist = 10;
				
                if (pCharInfo->MemorizedSpells[nGem] != 0xFFFFFFFF)
				{
					switch (pCharInfo->MemorizedSpells[nGem])
					{
					case 7864:
						trackingDist += 50;
						break;
					case 7865:
						trackingDist += 100;
						break;
					case 7866:
						trackingDist += 150;
						break;
					case 7867:
						trackingDist += 200;
						break;
					case 7868:
						trackingDist += 250;
						break;
					case 7869:
						trackingDist += 300;
						break;
					case 7870:
						trackingDist += 350;
						break;
					case 7871:
						trackingDist += 450;
						break;
					case 7872:
						trackingDist += 500;
						break;
					case 7873:
						trackingDist += 750;
						break;
					case 7874:
						trackingDist += 1000;
						break;
					}
				}
                
			}
		}
	}
    */
	return trackingDist;
}
inline void DeleteLabel(PMAPLABEL pLabel)
{
    LabelMap[pLabel]=0;
    if (pLabel->pNext)
        pLabel->pNext->pPrev=pLabel->pPrev;
    else
        pLabelListTail=pLabel->pPrev;
    if (pLabel->pPrev)
        pLabel->pPrev->pNext=pLabel->pNext;
    else
        pLabelList=pLabel->pNext;
    delete pLabel;
}

inline PMAPSPAWN InitSpawn()
{
    PMAPSPAWN pSpawn=new MAPSPAWN;
    pSpawn->pLast=0;
    pSpawn->pNext=pActiveSpawns;
    if (pActiveSpawns)
        pActiveSpawns->pLast=pSpawn;
    pActiveSpawns=pSpawn;
    return pSpawn;    
}

inline void DeleteSpawn(PMAPSPAWN pMapSpawn)
{
	if (pMapSpawn && pMapSpawn->pNext)
        pMapSpawn->pNext->pLast=pMapSpawn->pLast;
	if (pMapSpawn && pMapSpawn->pLast)
        pMapSpawn->pLast->pNext=pMapSpawn->pNext;
	else {
		if (pMapSpawn) {
        pActiveSpawns=pMapSpawn->pNext;
		}
	}
	if (pMapSpawn) {
    delete pMapSpawn;
}
}

PCHAR GenerateSpawnName(PSPAWNINFO pSpawn, PCHAR NameString);
DWORD GetSpawnColor(eSpawnType Type, PSPAWNINFO pSpawn);
PMAPLABEL GenerateLabel(PMAPSPAWN pMapSpawn, DWORD Color);
PMAPLINE GenerateVector(PMAPSPAWN pMapSpawn);
BOOL CanDisplaySpawn(eSpawnType Type, PSPAWNINFO pSpawn);

VOID MapInit()
{
    for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
    {
        pCastRadius[i]=0;
        pSpellRadius[i]=0;
        pTargetRadius[i]=0;
        pTargetMelee[i]=0;
    }
}

PMAPSPAWN AddSpawn(PSPAWNINFO pNewSpawn,BOOL ExplicitAllow)
{
    char buf[MAX_STRING] = {0};
    eSpawnType Type=GetSpawnType(pNewSpawn);
    // apply map filter
    if (!ExplicitAllow && !CanDisplaySpawn(Type,pNewSpawn))
        return 0;
    // add spawn to list

	if (PMAPSPAWN pMapSpawn = InitSpawn()) {
    if (pNewSpawn->Type!=FAKESPAWNTYPE)
        SpawnMap[pNewSpawn->SpawnID]=pMapSpawn;
    pMapSpawn->SpawnType=Type;
    pMapSpawn->pSpawn=pNewSpawn;
    pMapSpawn->pMapLabel = GenerateLabel(pMapSpawn,GetSpawnColor(Type,pNewSpawn));
    pMapSpawn->Explicit = ExplicitAllow;
    if (IsOptionEnabled(MAPFILTER_Vector))
        pMapSpawn->pVector= GenerateVector(pMapSpawn);
    else
        pMapSpawn->pVector=0;

    pMapSpawn->Highlight=false;

#if 0
    //Debugging
    if (Type == SPAWN_CORPSE || Type == ITEM )
    {
        sprintf(buf, "AddSpawn(Corpse or Item): Name: %s, Type: %d, BodyType: %d",
            pMapSpawn->pSpawn->Name, pMapSpawn->pSpawn->Type, GetBodyType(pMapSpawn->pSpawn) );
        DebugSpew(buf);
    }
#endif

    return pMapSpawn;
}
	return 0;
}

void RemoveSpawn(PMAPSPAWN pMapSpawn)
{
	if (pMapSpawn && pMapSpawn->pMapLabel) {
		if (pMapSpawn->pMapLabel->Label)
    free(pMapSpawn->pMapLabel->Label);
    DeleteLabel(pMapSpawn->pMapLabel);
	}
	if (pMapSpawn && pMapSpawn->pVector)
    {
        DeleteLine(pMapSpawn->pVector);
        pMapSpawn->pVector=0;
    }
	if (pMapSpawn && pMapSpawn->pSpawn && pMapSpawn->pSpawn->Type == FAKESPAWNTYPE)
        delete pMapSpawn->pSpawn;
	else {
		if (pMapSpawn && pMapSpawn->pSpawn) {
        SpawnMap[pMapSpawn->pSpawn->SpawnID]=0;
		}
	}
	if (pMapSpawn && pMapSpawn == pLastTarget) {
        pLastTarget=0;
	}
	if (pMapSpawn) {
    DeleteSpawn(pMapSpawn);
}
}

bool RemoveSpawn(PSPAWNINFO pSpawn)
{
	if (pSpawn) {
		if (SpawnMap.find(pSpawn->SpawnID) != SpawnMap.end()) {
    PMAPSPAWN pMapSpawn=SpawnMap[pSpawn->SpawnID];
        RemoveSpawn(pMapSpawn);
        return true;
    }
	}
    return false;
}

void AddGroundItem(PGROUNDITEM pGroundItem)
{
    PSPAWNINFO pFakeSpawn=new SPAWNINFO;
    memset(pFakeSpawn,0,sizeof(SPAWNINFO));
    GetFriendlyNameForGroundItem(pGroundItem,pFakeSpawn->Name);
    pFakeSpawn->X = pGroundItem->X;
    pFakeSpawn->Y = pGroundItem->Y;
    pFakeSpawn->Z = pGroundItem->Z;

    pFakeSpawn->HPCurrent = 1;
    pFakeSpawn->HPMax = 1;
    pFakeSpawn->Heading=pGroundItem->Heading;
    pFakeSpawn->Race = pGroundItem->DropID;

    pFakeSpawn->Type=FAKESPAWNTYPE;
    PMAPSPAWN pMapSpawn=AddSpawn(pFakeSpawn);
    if (pMapSpawn)
        GroundItemMap[pGroundItem->DropID]=pMapSpawn;
    else
        delete pFakeSpawn;
}

void RemoveGroundItem(PGROUNDITEM pGroundItem)
{
    PMAPSPAWN pMapSpawn=GroundItemMap[pGroundItem->DropID];
    if (pMapSpawn)
    {
        GroundItemMap[pGroundItem->DropID]=0;
        RemoveSpawn(pMapSpawn);
    }
}


void MapGenerate()
{
    if (!IsOptionEnabled(MAPFILTER_All))
        return;
    PSPAWNINFO pSpawn=(PSPAWNINFO)pSpawnList;
    while(pSpawn)
    {
        AddSpawn(pSpawn);
        pSpawn=pSpawn->pNext;
    }    
    if (!IsOptionEnabled(MAPFILTER_Ground))
        return;
    PGROUNDITEM pItem=*(PGROUNDITEM*)pItemList;
    while(pItem)
    {
        pItem=pItem->pNext;
    }    
}

void MapClear()
{
    GroundItemMap.clear();
    SpawnMap.clear();

    while(pActiveSpawns)
    {
        PMAPSPAWN pNextActive=pActiveSpawns->pNext;

        PMAPLABEL pLabel=pActiveSpawns->pMapLabel;
        DebugTry(free(pLabel->Label));
        DeleteLabel(pLabel);

        if (pActiveSpawns->pVector)
        {
            DeleteLine(pActiveSpawns->pVector);
            pActiveSpawns->pVector=0;
        }

        if (pActiveSpawns->pSpawn->Type==FAKESPAWNTYPE) // fake!
        {
            delete pActiveSpawns->pSpawn;
        }

        DeleteSpawn(pActiveSpawns);
        pActiveSpawns=pNextActive;
    }

    pLastTarget=0;

    if (pTargetLine)
    {
        DeleteLine(pTargetLine);
        pTargetLine=0;
    }

    if (pCastRadius[0])
    {
        for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
        {
            DeleteLine(pCastRadius[i]);
            pCastRadius[i]=0;
        }
    }
    if (pSpellRadius[0])
    {
        for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
        {
            DeleteLine(pSpellRadius[i]);
            pSpellRadius[i]=0;
        }
    }
    if (pTargetRadius[0])
    {
        for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
        {
            DeleteLine(pTargetRadius[i]);
            pTargetRadius[i]=0;
        }
    }
    if (pTargetMelee[0])
    {
        for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
        {
            DeleteLine(pTargetMelee[i]);
            pTargetMelee[i]=0;
        }
    }
}

void MapUpdate()
{
    char buf[MAX_STRING] = {0};
    PCHARINFO pCharInfo=GetCharInfo();
    if (!pCharInfo)
        return;
    eSpawnType Type;
    PMAPSPAWN pMapSpawn;
    PMAPSPAWN pOldLastTarget = NULL; 
    bool bTargetChanged = false; 

    pOldLastTarget = pLastTarget; 
    if (pLastTarget && pLastTarget->pSpawn!=(PSPAWNINFO)pTarget)
    {
        if (pLastTarget->pSpawn==&EnviroTarget || !CanDisplaySpawn(pLastTarget->SpawnType,pLastTarget->pSpawn))
        {
            RemoveSpawn(pLastTarget);
        }
        pLastTarget=0;
        bTargetChanged=true;
    }

    if (pTarget && IsOptionEnabled(MAPFILTER_Target))
    {
        pLastTarget=SpawnMap[((PSPAWNINFO)pTarget)->SpawnID];
        if (!pLastTarget)
        {
            pLastTarget=AddSpawn((PSPAWNINFO)pTarget);
        }
    }

    pMapSpawn=pActiveSpawns;
#ifndef zWarp
    gSpewToFile=1;
#endif

	DWORD trackingAmount = GetTrackingAmount();

    while(pMapSpawn)
    {
#if 0
        // Debugging
        DWORD BodyType=GetBodyType(pMapSpawn->pSpawn);
        if ( pMapSpawn->pSpawn->Type == SPAWN_PLAYER )
        {
            if ( BodyType != 1 )
            {
                sprintf(buf, "MapUpdate: Name: %s, Type: %d, BodyType: %d",
                    pMapSpawn->pSpawn->Name, pMapSpawn->pSpawn->Type, BodyType );
                DebugSpew(buf);
            }
        }
        else if ( pMapSpawn->pSpawn->Type == SPAWN_NPC )
        {
            if ( BodyType != 1 && BodyType != 11 &&
                BodyType != 33 && BodyType != 65 &&
                BodyType != 66 && BodyType != 67 &&
                BodyType != 21 && BodyType != 23 &&
                BodyType != 34 && BodyType != 3  &&
                BodyType != 24 )
            {
                sprintf(buf, "MapUpdate: Name: %s, Type: %d, BodyType: %d",
                    pMapSpawn->pSpawn->Name, pMapSpawn->pSpawn->Type, BodyType );
                DebugSpew(buf);
            }
        }
        else 
        {
            sprintf(buf, "MapUpdate: Name: %s, Type: %d, BodyType: %d",
                pMapSpawn->pSpawn->Name, pMapSpawn->pSpawn->Type, BodyType );
            DebugSpew(buf);
        }

        //End Debugging
#endif

        //Starting New Checks
        if (!pMapSpawn->Explicit && !CanDisplaySpawn(GetSpawnType(pMapSpawn->pSpawn),pMapSpawn->pSpawn)) 
        {
            PMAPSPAWN pNext=pMapSpawn->pNext;
            RemoveSpawn(pMapSpawn);
            pMapSpawn=pNext;
            continue;
        }
        //End New Checks

        pMapSpawn->pMapLabel->Location.X = -pMapSpawn->pSpawn->X;
        pMapSpawn->pMapLabel->Location.Y = -pMapSpawn->pSpawn->Y;
        pMapSpawn->pMapLabel->Location.Z = pMapSpawn->pSpawn->Z;

        if (pMapSpawn->Highlight)
        {
            pMapSpawn->pMapLabel->Color.ARGB=HighlightColor;
        }
        else
        {
            Type=GetSpawnType(pMapSpawn->pSpawn);
            if (Type!=pMapSpawn->SpawnType || ((pMapSpawn==pOldLastTarget) && bTargetChanged))
            {
                if (!CanDisplaySpawn(Type,pMapSpawn->pSpawn))
                {
                    PMAPSPAWN pNext=pMapSpawn->pNext;
                    RemoveSpawn(pMapSpawn);
                    pMapSpawn=pNext;
                    continue;
                }

                pMapSpawn->SpawnType=Type;
                free(pMapSpawn->pMapLabel->Label);
                pMapSpawn->pMapLabel->Label=GenerateSpawnName(pMapSpawn->pSpawn,MapNameString);

            }
            pMapSpawn->pMapLabel->Color.ARGB=GetSpawnColor(pMapSpawn->SpawnType,pMapSpawn->pSpawn);
        }

        if (pMapSpawn->pVector)
        {
            if (pMapSpawn->pSpawn->SpeedRun>0) {
                pMapSpawn->pVector->Start.X = -pMapSpawn->pSpawn->X;
                pMapSpawn->pVector->Start.Y = -pMapSpawn->pSpawn->Y;
                pMapSpawn->pVector->Start.Z = pMapSpawn->pSpawn->Z;
                pMapSpawn->pVector->End.X = -pMapSpawn->pSpawn->X-pMapSpawn->pSpawn->SpeedX*4;
                pMapSpawn->pVector->End.Y = -pMapSpawn->pSpawn->Y-pMapSpawn->pSpawn->SpeedY*4;
                pMapSpawn->pVector->End.Z = pMapSpawn->pSpawn->Z+pMapSpawn->pSpawn->SpeedZ*4;
            } else {
                pMapSpawn->pVector->Start.X = -pMapSpawn->pSpawn->X;
                pMapSpawn->pVector->Start.Y = -pMapSpawn->pSpawn->Y;
                pMapSpawn->pVector->Start.Z = pMapSpawn->pSpawn->Z;
                pMapSpawn->pVector->End.X =-pMapSpawn->pSpawn->X-sinf(pMapSpawn->pSpawn->Heading/256.0f*(FLOAT)PI)*4;
                pMapSpawn->pVector->End.Y =-pMapSpawn->pSpawn->Y-cosf(pMapSpawn->pSpawn->Heading/256.0f*(FLOAT)PI)*4;
                pMapSpawn->pVector->End.Z =pMapSpawn->pSpawn->Z;
            }
        }

		if (pMapSpawn && pMapSpawn->pMapLabel)
		{
			GenerateSpawnLabel(pMapSpawn, pMapSpawn->pMapLabel->Color.ARGB, trackingAmount);
		}

        pMapSpawn=pMapSpawn->pNext;
    }

    if (IsOptionEnabled(MAPFILTER_CastRadius))
    {
        unsigned long Angle=0;
        for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++,Angle+=CASTRADIUS_ANGLESIZE)
        {
            if (!pCastRadius[i])
            {
                pCastRadius[i]=InitLine();
                pCastRadius[i]->Layer=2;
            }

            pCastRadius[i]->Color.ARGB=MapFilterOptions[MAPFILTER_CastRadius].Color;
            pCastRadius[i]->Start.Z=pCharInfo->pSpawn->Z;
            pCastRadius[i]->End.Z=pCharInfo->pSpawn->Z;
            pCastRadius[i]->Start.X=-pCharInfo->pSpawn->X + (FLOAT)MapFilterOptions[MAPFILTER_CastRadius].Enabled*cosf((FLOAT)Angle/180.0f*(FLOAT)PI);
            pCastRadius[i]->Start.Y=-pCharInfo->pSpawn->Y + (FLOAT)MapFilterOptions[MAPFILTER_CastRadius].Enabled*sinf((FLOAT)Angle/180.0f*(FLOAT)PI);;
            pCastRadius[i]->End.X=-pCharInfo->pSpawn->X + (FLOAT)MapFilterOptions[MAPFILTER_CastRadius].Enabled*cosf((FLOAT)(Angle+CASTRADIUS_ANGLESIZE)/180.0f*(FLOAT)PI);
            pCastRadius[i]->End.Y=-pCharInfo->pSpawn->Y + (FLOAT)MapFilterOptions[MAPFILTER_CastRadius].Enabled*sinf((FLOAT)(Angle+CASTRADIUS_ANGLESIZE)/180.0f*(FLOAT)PI);
        }
    }
    else if (pCastRadius[0])
    {
        for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
        {
            DeleteLine(pCastRadius[i]);
            pCastRadius[i]=0;
        }
    }

    if (IsOptionEnabled(MAPFILTER_SpellRadius))
    {
        unsigned long Angle=0;
        for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++,Angle+=CASTRADIUS_ANGLESIZE)
        {
            if (!pSpellRadius[i])
            {
                pSpellRadius[i]=InitLine();
                pSpellRadius[i]->Layer=2;
            }

            pSpellRadius[i]->Color.ARGB=MapFilterOptions[MAPFILTER_SpellRadius].Color;
            pSpellRadius[i]->Start.Z=pCharInfo->pSpawn->Z;
            pSpellRadius[i]->End.Z=pCharInfo->pSpawn->Z;
            pSpellRadius[i]->Start.X=-pCharInfo->pSpawn->X + (FLOAT)MapFilterOptions[MAPFILTER_SpellRadius].Enabled*cosf((FLOAT)Angle/180.0f*(FLOAT)PI);
            pSpellRadius[i]->Start.Y=-pCharInfo->pSpawn->Y + (FLOAT)MapFilterOptions[MAPFILTER_SpellRadius].Enabled*sinf((FLOAT)Angle/180.0f*(FLOAT)PI);;
            pSpellRadius[i]->End.X=-pCharInfo->pSpawn->X + (FLOAT)MapFilterOptions[MAPFILTER_SpellRadius].Enabled*cosf((FLOAT)(Angle+CASTRADIUS_ANGLESIZE)/180.0f*(FLOAT)PI);
            pSpellRadius[i]->End.Y=-pCharInfo->pSpawn->Y + (FLOAT)MapFilterOptions[MAPFILTER_SpellRadius].Enabled*sinf((FLOAT)(Angle+CASTRADIUS_ANGLESIZE)/180.0f*(FLOAT)PI);
        }
    }
    else if (pSpellRadius[0])
    {
        for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
        {
            DeleteLine(pSpellRadius[i]);
            pSpellRadius[i]=0;
        }
    }

    if (IsOptionEnabled(MAPFILTER_Group))
    {
        PCHARINFO pChar=GetCharInfo();
        for (unsigned long i=1; i<6; i++)
        {
            if (pChar->pGroupInfo && pChar->pGroupInfo->pMember[i])
            {
                CHAR Name[MAX_STRING]={0};
                GetCXStr(pChar->pGroupInfo->pMember[i]->pName,Name,MAX_STRING);
                PSPAWNINFO pSpawn=(PSPAWNINFO)GetSpawnByName(Name);
                if (pSpawn)
                    if (pMapSpawn=SpawnMap[pSpawn->SpawnID])
                    {
                        pMapSpawn->pMapLabel->Color.ARGB=MapFilterOptions[MAPFILTER_Group].Color;
                    }
            }
        }
    }

    if (pLastTarget)
    {
        pLastTarget->pMapLabel->Color.ARGB=MapFilterOptions[MAPFILTER_Target].Color;
        free(pLastTarget->pMapLabel->Label);
        pLastTarget->pMapLabel->Label=GenerateSpawnName(pLastTarget->pSpawn,MapTargetNameString);            

        if (IsOptionEnabled(MAPFILTER_TargetLine))
        {
            if (!pTargetLine)
            {
                pTargetLine=InitLine();
                pTargetLine->Layer=2;
            }
            pTargetLine->Color.ARGB=MapFilterOptions[MAPFILTER_Target].Color;
            pTargetLine->Start.X=-pCharInfo->pSpawn->X;
            pTargetLine->Start.Y=-pCharInfo->pSpawn->Y;
            pTargetLine->Start.Z=pCharInfo->pSpawn->Z;

            pTargetLine->End.X=-((PSPAWNINFO)pTarget)->X;
            pTargetLine->End.Y=-((PSPAWNINFO)pTarget)->Y;
            pTargetLine->End.Z=((PSPAWNINFO)pTarget)->Z;
        }
        else if (pTargetLine)
        {
            DeleteLine(pTargetLine);
            pTargetLine=0;
        }

        if (IsOptionEnabled(MAPFILTER_TargetRadius))
        {
            unsigned long Angle=0;
            for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++,Angle+=CASTRADIUS_ANGLESIZE)
            {
                if (!pTargetRadius[i])
                {
                    pTargetRadius[i]=InitLine();
                    pTargetRadius[i]->Layer=2;
                }

                pTargetRadius[i]->Color.ARGB=MapFilterOptions[MAPFILTER_TargetRadius].Color;
                pTargetRadius[i]->Start.Z=((PSPAWNINFO)pTarget)->Z;
                pTargetRadius[i]->End.Z=((PSPAWNINFO)pTarget)->Z;
                pTargetRadius[i]->Start.X=-((PSPAWNINFO)pTarget)->X + (FLOAT)MapFilterOptions[MAPFILTER_TargetRadius].Enabled*cosf((FLOAT)Angle/180.0f*(FLOAT)PI);
                pTargetRadius[i]->Start.Y=-((PSPAWNINFO)pTarget)->Y + (FLOAT)MapFilterOptions[MAPFILTER_TargetRadius].Enabled*sinf((FLOAT)Angle/180.0f*(FLOAT)PI);;
                pTargetRadius[i]->End.X=-((PSPAWNINFO)pTarget)->X + (FLOAT)MapFilterOptions[MAPFILTER_TargetRadius].Enabled*cosf((FLOAT)(Angle+CASTRADIUS_ANGLESIZE)/180.0f*(FLOAT)PI);
                pTargetRadius[i]->End.Y=-((PSPAWNINFO)pTarget)->Y + (FLOAT)MapFilterOptions[MAPFILTER_TargetRadius].Enabled*sinf((FLOAT)(Angle+CASTRADIUS_ANGLESIZE)/180.0f*(FLOAT)PI);
            }
        }
        else if (pTargetRadius[0])
        {
            for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
            {
                DeleteLine(pTargetRadius[i]);
                pTargetRadius[i]=0;
            }
        }

        if (IsOptionEnabled(MAPFILTER_TargetMelee))
        {
            unsigned long Angle=0;
            for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++,Angle+=CASTRADIUS_ANGLESIZE)
            {
                if (!pTargetMelee[i])
                {
                    pTargetMelee[i]=InitLine();
                    pTargetMelee[i]->Layer=2;
                }

                pTargetMelee[i]->Color.ARGB=MapFilterOptions[MAPFILTER_TargetMelee].Color;
                pTargetMelee[i]->Start.Z=((PSPAWNINFO)pTarget)->Z;
                pTargetMelee[i]->End.Z=((PSPAWNINFO)pTarget)->Z;
                FLOAT MaxMelee;
                if (MapFilterOptions[MAPFILTER_TargetMelee].Enabled==1)
                    MaxMelee=get_melee_range(pLocalPlayer,pTarget); 
                else
                    MaxMelee=get_melee_range(pTarget,pTarget);
                pTargetMelee[i]->Start.X=-((PSPAWNINFO)pTarget)->X + MaxMelee*cosf((FLOAT)Angle/180.0f*(FLOAT)PI);
                pTargetMelee[i]->Start.Y=-((PSPAWNINFO)pTarget)->Y + MaxMelee*sinf((FLOAT)Angle/180.0f*(FLOAT)PI);;
                pTargetMelee[i]->End.X=-((PSPAWNINFO)pTarget)->X + MaxMelee*cosf((FLOAT)(Angle+CASTRADIUS_ANGLESIZE)/180.0f*(FLOAT)PI);
                pTargetMelee[i]->End.Y=-((PSPAWNINFO)pTarget)->Y + MaxMelee*sinf((FLOAT)(Angle+CASTRADIUS_ANGLESIZE)/180.0f*(FLOAT)PI);
            }
        }
        else if (pTargetMelee[0])
        {
            for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
            {
                DeleteLine(pTargetMelee[i]);
                pTargetMelee[i]=0;
            }
        }
    }
    else
    {
        if (pTargetRadius[0])
        {
            for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
            {
                DeleteLine(pTargetRadius[i]);
                pTargetRadius[i]=0;
            }
        }

        if (pTargetMelee[0])
        {
            for (unsigned long i = 0 ; i < (360/CASTRADIUS_ANGLESIZE) ; i++)
            {
                DeleteLine(pTargetMelee[i]);
                pTargetMelee[i]=0;
            }
        }

        if (pTargetLine)
        {
            DeleteLine(pTargetLine);
            pTargetLine=0;
        }
    }
}


PMAPLABEL pActualLineList=0;
void MapAttach()
{
    if (pLabelList)
    {
        pActualLineList=pMap->pLabels;
        if (IsOptionEnabled(MAPFILTER_NormalLabels))
            pLabelListTail->pNext=pMap->pLabels;
        pMap->pLabels=pLabelList;
    }

    if (pLineList)
    {
        pLineListTail->pNext=pMap->pLines;
        pMap->pLines=pLineList;
    }
}

void MapDetach()
{
    if (pLabelList)
    {
        pMap->pLabels=pActualLineList;
        pLabelListTail->pNext=0;
    }
    if (pLineList)
    {
        pMap->pLines=pLineListTail->pNext;
        pLineListTail->pNext=0;
    }
}

bool MapSelectTarget()
{
    if (!pCurrentMapLabel)
        return false;

    DWORD TrackingDist = GetTrackingAmount();

    /*
	if (!pLocalPlayer || pLocalPlayer && !pLocalPlayer->Data.pSpawn ||
        pLocalPlayer && pLocalPlayer->Data.pSpawn && !pLocalPlayer->Data.pSpawn->GM)
       return false;
    */
    if (!pLocalPlayer || pLocalPlayer && !pLocalPlayer->Data.pSpawn)
        return false;

    PMAPSPAWN pMapSpawn=LabelMap[pCurrentMapLabel];
    if (!pMapSpawn)
        return false;

    /*
    if (!pLocalPlayer->Data.pSpawn->GM && !IsGroupMember(pMapSpawn->pSpawn) &&
        DistanceToSpawn(pLocalPlayer->Data.pSpawn, pMapSpawn->pSpawn) > TrackingDist)
        return false;
    */

    pTarget = (EQPlayer*)pMapSpawn->pSpawn;
    return true;

    if (pMapSpawn->SpawnType==ITEM)
    {
        EnviroTarget=*pMapSpawn->pSpawn;
        EnviroTarget.Type = SPAWN_NPC;
        //EnviroTarget.SpawnID=2999;
        //pTarget = (EQPlayer*)&EnviroTarget;                 
    }
    else
    {
        DWORD Flags=pWndMgr->GetKeyboardFlags();
        PCHARINFO pCharInfo=GetCharInfo();
        if (pCharInfo && Flags && MapSpecialClickString[Flags][0])
        {
            PCHAR Cmd=GenerateSpawnName(pMapSpawn->pSpawn,MapSpecialClickString[Flags]);
#ifndef ISXEQ
            DoCommand(pCharInfo->pSpawn,Cmd);
#else
            pISInterface->ExecuteCommand(Cmd);
#endif
            free(Cmd);
        }
        else
        {
            pTarget=(EQPlayer*)pMapSpawn->pSpawn;
        }
    }
    if(IsOptionEnabled(MAPFILTER_TargetPath))
        return true;
    return true;
}

DWORD MapHighlight(SEARCHSPAWN *pSearch)
{
    if (!pSearch)
    {
        PMAPSPAWN pMapSpawn=pActiveSpawns;
        while(pMapSpawn)
        {
            pMapSpawn->Highlight=false;
            pMapSpawn=pMapSpawn->pNext;
        }
        return 0;
    }
    else
    {
        PCHARINFO pCharInfo=GetCharInfo();
		if (!pCharInfo || !pCharInfo->pSpawn)
            return 0;
        PMAPSPAWN pMapSpawn=pActiveSpawns;
        unsigned long Count=0;
        while(pMapSpawn)
        {
            // update!
            if (SpawnMatchesSearch(pSearch,pCharInfo->pSpawn,pMapSpawn->pSpawn))
            {
                pMapSpawn->Highlight=true;
                Count++;
            }

            pMapSpawn=pMapSpawn->pNext;
        }
        return Count;
    }
}

PCHAR GenerateSpawnName(PSPAWNINFO pSpawn, PCHAR NameString)
{

    CHAR Name[MAX_STRING]={0};
    unsigned long outpos=0;
#define AddString(str) {strcpy(&Name[outpos],str);outpos+=strlen(&Name[outpos]);}
#define AddInt(yourint) {_itoa(yourint,&Name[outpos],10);outpos+=strlen(&Name[outpos]);}
#define AddFloat10th(yourfloat) {outpos+=sprintf(&Name[outpos],"%.1f",yourfloat);}
    for (unsigned long N = 0 ; NameString[N] ; N++)
    {
        if (NameString[N]=='%')
        {
            N++;
            switch(NameString[N])
            {
            case 'N':// cleaned up name
                strcpy(&Name[outpos],pSpawn->Name);
                CleanupName(&Name[outpos],FALSE);
                outpos+=strlen(&Name[outpos]);
                break;
            case 'n':// original name
                AddString(pSpawn->Name);
                break;
            case 'h':// current health %
                AddInt(pSpawn->HPCurrent);
                break;
            case 'i':
                AddInt(pSpawn->SpawnID);
                break;
            case 'x':
                AddFloat10th(pSpawn->X);
                break;
            case 'y':
                AddFloat10th(pSpawn->Y);
                break;
            case 'z':
                AddFloat10th(pSpawn->Z);
                break;
            case 'R':
                AddString(pEverQuest->GetRaceDesc(pSpawn->Race));
                break;
            case 'C':
                AddString(GetClassDesc(pSpawn->Class));
                break;
            case 'c':
                AddString(pEverQuest->GetClassThreeLetterCode(pSpawn->Class));
                break;
            case 'l':
                AddInt(pSpawn->Level);
                break;
            case '%':
                Name[outpos++]=NameString[N];
                break;
            }
        }
        else
            Name[outpos++]=NameString[N];
    }
	Name[outpos] = 0;

    PCHAR ret=(PCHAR)malloc(strlen(Name)+1);
    strcpy(ret,Name);
    return ret;
}

BOOL IsSpawnGroupMember(PSPAWNINFO pSpawn)
{
	for (unsigned long i = 1; i < 6; i++)
	{
		if (PCHARINFO pCharInfo = GetCharInfo())
		{
			if (pCharInfo->pSpawn)
			{
				if (pCharInfo->pSpawn->GM)
					return true;
				if (pCharInfo->pGroupInfo && pCharInfo->pGroupInfo->pMember[i] && pCharInfo->pGroupInfo->pMember[i]->pSpawn == pSpawn)
				{
					return true;
				}
			}
		}
	}
	return false;
}



BOOL CanDisplaySpawn(eSpawnType Type, PSPAWNINFO pSpawn)
{

    if (!pSpawn)
        return FALSE;

    if ((pSpawn==(PSPAWNINFO)pTarget) && IsOptionEnabled(MAPFILTER_Target))
    {
        return TRUE;
    }

    if (IsOptionEnabled(MAPFILTER_Custom))
    {
        if (PCHARINFO pCharInfo=GetCharInfo())
			if (pCharInfo->pSpawn)
            return SpawnMatchesSearch(&MapFilterCustom,pCharInfo->pSpawn,pSpawn);
    }
    switch(Type)
    {
    case PC:
        return IsOptionEnabled(MAPFILTER_PC);
    case NPC:
        if (IsOptionEnabled(MAPFILTER_Named))
            return IsNamed(pSpawn);
        else
            return IsOptionEnabled(MAPFILTER_NPC);
    case CORPSE:
        if(pSpawn->Deity == 0)
            return (IsOptionEnabled(MAPFILTER_NPCCorpse) && IsOptionEnabled(MAPFILTER_Corpse));
        else
            return (IsOptionEnabled(MAPFILTER_PCCorpse) && IsOptionEnabled(MAPFILTER_Corpse));
    case ITEM:
        return IsOptionEnabled(MAPFILTER_Ground);
    case UNTARGETABLE:
        return IsOptionEnabled(MAPFILTER_Untargetable);
    case TIMER:
        return IsOptionEnabled(MAPFILTER_Timer);
    case TRAP:
        return IsOptionEnabled(MAPFILTER_Trap);
    case TRIGGER:
        return IsOptionEnabled(MAPFILTER_Trigger);
    case CHEST:
        return IsOptionEnabled(MAPFILTER_Chest);
    case PET:
        return IsOptionEnabled(MAPFILTER_Pet);
    case MOUNT:
        return IsOptionEnabled(MAPFILTER_Mount);
    case AURA:
        return IsOptionEnabled(MAPFILTER_Aura);
    case OBJECT:
        return IsOptionEnabled(MAPFILTER_Object);
    case BANNER:
        return IsOptionEnabled(MAPFILTER_Banner);
    case CAMPFIRE:
        return IsOptionEnabled(MAPFILTER_Campfire);
    case MERCENARY:
        return IsOptionEnabled(MAPFILTER_Mercenary);
    case FLYER:
        return FALSE; // they have no location data, no use trying to display them
    }
    return TRUE;
}

inline DWORD GetSpawnColor(eSpawnType Type, PSPAWNINFO pSpawn)
{
    PCHARINFO pChar=GetCharInfo();
    if (!pChar)
        return 0;

    switch(Type)
    {
    case PC:
        if (IsOptionEnabled(MAPFILTER_PCConColor))
            return ConColorToARGB(ConColor(pSpawn));
        return MapFilterOptions[MAPFILTER_PC].Color;
    case NPC:
        if (IsOptionEnabled(MAPFILTER_NPCConColor))
            return ConColorToARGB(ConColor(pSpawn));
        return MapFilterOptions[MAPFILTER_NPC].Color;
    case CORPSE:
        if(pSpawn->Deity == 0)
            return MapFilterOptions[MAPFILTER_NPCCorpse].Color;
        else
            return MapFilterOptions[MAPFILTER_PCCorpse].Color;
    case UNTARGETABLE:
        return MapFilterOptions[MAPFILTER_Untargetable].Color;
    case CHEST:
        return MapFilterOptions[MAPFILTER_Chest].Color;
    case TRIGGER:
        return MapFilterOptions[MAPFILTER_Trigger].Color;
    case TIMER:
        return MapFilterOptions[MAPFILTER_Timer].Color;
    case TRAP:
        return MapFilterOptions[MAPFILTER_Trap].Color;
    case ITEM:
        return MapFilterOptions[MAPFILTER_Ground].Color;
    case MOUNT:
        return MapFilterOptions[MAPFILTER_Mount].Color;
    case PET:
        return MapFilterOptions[MAPFILTER_Pet].Color;
    case AURA:
        return MapFilterOptions[MAPFILTER_Aura].Color;
    case OBJECT:
        return MapFilterOptions[MAPFILTER_Object].Color;
    case BANNER:
        return MapFilterOptions[MAPFILTER_Banner].Color;
    case CAMPFIRE:
        return MapFilterOptions[MAPFILTER_Campfire].Color;
    case MERCENARY:
        return MapFilterOptions[MAPFILTER_Mercenary].Color;
    }
    return 0;
}


void GenerateSpawnLabel(PMAPSPAWN& pMapSpawn, DWORD Color, DWORD TrackingDist)
{

    DWORD secondaryDist = 50 + 1.5 * TrackingDist;

    if (!pLocalPlayer || pLocalPlayer && !pLocalPlayer->Data.pSpawn ||
        pLocalPlayer && pLocalPlayer->Data.pSpawn && !pLocalPlayer->Data.pSpawn->GM && pMapSpawn && pMapSpawn->pSpawn && !IsGroupMember(pMapSpawn->pSpawn) && DistanceToSpawn(pLocalPlayer->Data.pSpawn, pMapSpawn->pSpawn) > secondaryDist)
    {
        pMapSpawn->pMapLabel->Layer = 4;
        return;
    }
	pMapSpawn->pMapLabel->Location.X = -pMapSpawn->pSpawn->X;
	pMapSpawn->pMapLabel->Location.Y = -pMapSpawn->pSpawn->Y;
	pMapSpawn->pMapLabel->Location.Z = pMapSpawn->pSpawn->Z;
	pMapSpawn->pMapLabel->Layer = 2;
	pMapSpawn->pMapLabel->Size = 3;

    // players always show up 

	if (!pLocalPlayer->Data.pSpawn->GM && !IsGroupMember(pMapSpawn->pSpawn) && DistanceToSpawn(pLocalPlayer->Data.pSpawn, pMapSpawn->pSpawn) > TrackingDist) {
		if (pMapSpawn->pMapLabel->Label)
			free(pMapSpawn->pMapLabel->Label);
		pMapSpawn->pMapLabel->Label = GenerateSpawnName(pMapSpawn->pSpawn, pMapSpawn->pSpawn->Type == SPAWN_PLAYER ? " " : "Mob");
	}
	else
	{
		if(pMapSpawn && pMapSpawn->pSpawn && pMapSpawn->pSpawn->Type == SPAWN_PLAYER)
			pMapSpawn->pMapLabel->Layer = 3;
		if (pMapSpawn->pMapLabel->Label)
			free(pMapSpawn->pMapLabel->Label);
		pMapSpawn->pMapLabel->Label = GenerateSpawnName(pMapSpawn->pSpawn, MapNameString);
	}
	pMapSpawn->pMapLabel->Color.ARGB = Color;
	pMapSpawn->pMapLabel->Width = 20;
	pMapSpawn->pMapLabel->Height = 14;
	pMapSpawn->pMapLabel->unk_0x2c = 0;
	pMapSpawn->pMapLabel->unk_0x30 = 0;
	return;
}

PMAPLABEL GenerateLabel(PMAPSPAWN pMapSpawn, DWORD Color)
{
    PMAPLABEL pLabel=InitLabel(pMapSpawn);
    pLabel->Location.X = -pMapSpawn->pSpawn->X;
    pLabel->Location.Y = -pMapSpawn->pSpawn->Y;
    pLabel->Location.Z = pMapSpawn->pSpawn->Z;
    pLabel->Layer = 2;
    pLabel->Size = 3;

    
	if (!pLocalPlayer || pLocalPlayer && !pLocalPlayer->Data.pSpawn ||
        pLocalPlayer && pLocalPlayer->Data.pSpawn && !pLocalPlayer->Data.pSpawn->GM && pMapSpawn && pMapSpawn->pSpawn && pMapSpawn->pSpawn->Type != SPAWN_PLAYER && !IsGroupMember(pMapSpawn->pSpawn)) {
			pLabel->Label = GenerateSpawnName(pMapSpawn->pSpawn, pMapSpawn->pSpawn->Type == SPAWN_PLAYER ? " " : "Mob");
	}
	else
	{
		pLabel->Label = GenerateSpawnName(pMapSpawn->pSpawn, MapNameString);
		if (pMapSpawn && pMapSpawn->pSpawn && pMapSpawn->pSpawn->Type == SPAWN_PLAYER)
			pLabel->Layer = 3;
	}
    pLabel->Color.ARGB = Color;
    pLabel->Width = 20;
    pLabel->Height= 14;
    pLabel->unk_0x2c = 0;
    pLabel->unk_0x30 = 0;
    return pLabel;
}

PMAPLINE GenerateVector(PMAPSPAWN pMapSpawn)
{

    PMAPLINE pNewLine = InitLine();

    if (pMapSpawn->pSpawn->SpeedRun>0) {
        pNewLine->Start.X = -pMapSpawn->pSpawn->X;
        pNewLine->Start.Y = -pMapSpawn->pSpawn->Y;
        pNewLine->Start.Z = pMapSpawn->pSpawn->Z;
        pNewLine->End.X = -pMapSpawn->pSpawn->X-pMapSpawn->pSpawn->SpeedX*4;
        pNewLine->End.Y = -pMapSpawn->pSpawn->Y-pMapSpawn->pSpawn->SpeedY*4;
        pNewLine->End.Z = pMapSpawn->pSpawn->Z+pMapSpawn->pSpawn->SpeedZ*4;
    } else {
        pNewLine->Start.X = -pMapSpawn->pSpawn->X;
        pNewLine->Start.Y = -pMapSpawn->pSpawn->Y;
        pNewLine->Start.Z = pMapSpawn->pSpawn->Z;
        pNewLine->End.X =-pMapSpawn->pSpawn->X-sinf(pMapSpawn->pSpawn->Heading/256.0f*(FLOAT)PI)*4;
        pNewLine->End.Y =-pMapSpawn->pSpawn->Y-cosf(pMapSpawn->pSpawn->Heading/256.0f*(FLOAT)PI)*4;
        pNewLine->End.Z =pMapSpawn->pSpawn->Z;
    }



    pNewLine->Layer = 2;
    pNewLine->Color = pMapSpawn->pMapLabel->Color;

    return pNewLine;    
}

DWORD MapHide(SEARCHSPAWN &Search)
{
    PCHARINFO pCharInfo=GetCharInfo();
	if (!pCharInfo || !pCharInfo->pSpawn)
        return 0;
    PMAPSPAWN pMapSpawn=pActiveSpawns;
    unsigned long Count=0;
    while(pMapSpawn)
    {
        if (pMapSpawn->pSpawn->Type!=FAKESPAWNTYPE && SpawnMatchesSearch(&Search,pCharInfo->pSpawn,pMapSpawn->pSpawn))
        {
            PMAPSPAWN pNext=pMapSpawn->pNext;
            RemoveSpawn(pMapSpawn);
            pMapSpawn=pNext;
            Count++;
        }
        else
            pMapSpawn=pMapSpawn->pNext;
    }
    return Count;
}

DWORD MapShow(SEARCHSPAWN &Search)
{
    PCHARINFO pCharInfo=GetCharInfo();
    if (!pCharInfo)
        return 0;

    PSPAWNINFO pSpawn=(PSPAWNINFO)pSpawnList;
    unsigned long Count=0;
    while(pSpawn)
    {
        if (!SpawnMap[pSpawn->SpawnID] && SpawnMatchesSearch(&Search,pCharInfo->pSpawn,pSpawn))
        {
            AddSpawn(pSpawn,true);
            Count++;
        }
        pSpawn=pSpawn->pNext;
    }
    return Count;
}

#ifndef ISXEQ
BOOL dataMapSpawn(PCHAR szIndex, MQ2TYPEVAR &Ret)
#else
bool dataMapSpawn(int argc, char *argv[], MQ2TYPEVAR &Ret)
#endif
{
    if (!pCurrentMapLabel)
        return false;
    if (PMAPSPAWN pMapSpawn=LabelMap[pCurrentMapLabel])
    {
        Ret.Ptr=pMapSpawn->pSpawn;
        Ret.Type=pSpawnType;
        return true;
    }
    return false;
}

