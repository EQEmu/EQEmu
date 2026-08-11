#!/usr/bin/env python3
"""
Validate EverQuest achievement resource files and emit deterministic MySQL SQL.

By default, only client presentation data is imported. Progression selection
options additionally emit criteria only where the resource structure provides
an exact, independently verifiable mapping to a server event. All other
criteria, rewards, cast restrictions, and character state remain outside this
tool's scope.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import re
from dataclasses import dataclass, field
from pathlib import Path
import sys
from typing import Callable, Iterable, List, Mapping, Optional, Sequence, Tuple


UINT8_MAX = (1 << 8) - 1
UINT32_MAX = (1 << 32) - 1
INSERT_BATCH_SIZE = 250


# Stable EQ::skills::SkillType IDs from common/skills.h. Client component IDs
# are presentation identities and are not interchangeable with these IDs.
TRADESKILL_SKILL_IDS = {
    "Fishing": 55,
    "Poisonmaking": 56,
    "Tinkering": 57,
    "Research": 58,
    "Alchemy": 59,
    "Baking": 60,
    "Tailoring": 61,
    "Smithing": 63,
    "Fletching": 64,
    "Brewing": 65,
    "Jewelcrafting": 68,
    "Pottery": 69,
}

# Stable EQ class IDs from common/classes.h. General / Skills definitions carry
# the class only in a type-3 presentation component, so the importer validates
# that text against this explicit server mapping before emitting policy.
CLASS_IDS = {
    "Warrior": 1,
    "Cleric": 2,
    "Paladin": 3,
    "Ranger": 4,
    "ShadowKnight": 5,
    "Druid": 6,
    "Monk": 7,
    "Bard": 8,
    "Rogue": 9,
    "Shaman": 10,
    "Necromancer": 11,
    "Wizard": 12,
    "Magician": 13,
    "Enchanter": 14,
    "Beastlord": 15,
    "Berserker": 16,
}

# Stable EQ::skills::SkillType IDs from common/skills.h. These names are the
# exact spellings used by the client achievement components (including the
# client's "Channelling" spelling).
SKILL_CAP_SKILL_IDS = {
    "1H Blunt": 0,
    "1H Slashing": 1,
    "2H Blunt": 2,
    "2H Slashing": 3,
    "Abjuration": 4,
    "Alteration": 5,
    "Apply Poison": 6,
    "Archery": 7,
    "Backstab": 8,
    "Bind Wound": 9,
    "Bash": 10,
    "Block": 11,
    "Brass Instruments": 12,
    "Channelling": 13,
    "Conjuration": 14,
    "Defense": 15,
    "Disarm": 16,
    "Disarm Traps": 17,
    "Divination": 18,
    "Dodge": 19,
    "Double Attack": 20,
    "Dragon Punch": 21,
    "Dual Wield": 22,
    "Eagle Strike": 23,
    "Evocation": 24,
    "Feign Death": 25,
    "Flying Kick": 26,
    "Forage": 27,
    "Hand To Hand": 28,
    "Hide": 29,
    "Kick": 30,
    "Meditate": 31,
    "Mend": 32,
    "Offense": 33,
    "Parry": 34,
    "Pick Lock": 35,
    "1H Piercing": 36,
    "Riposte": 37,
    "Round Kick": 38,
    "Safe Fall": 39,
    "Sense Heading": 40,
    "Singing": 41,
    "Sneak": 42,
    "Pick Pockets": 47,
    "Stringed Instruments": 48,
    "Swimming": 50,
    "Throwing": 51,
    "Tiger Claw": 52,
    "Tracking": 53,
    "Wind Instruments": 54,
    "Sense Traps": 62,
    "Alcohol Tolerance": 66,
    "Begging": 67,
    "Percussion Instruments": 70,
    "Intimidation": 71,
    "Taunt": 73,
    "Frenzy": 74,
    "Triple Attack": 76,
    "2H Piercing": 77,
}

HUNTER_CATEGORY_NAMES = frozenset(("hunter", "hunts"))

DON_MAX_HISTORICAL_ZONE_ID = 345
DON_EXCLUDED_ZONE_IDS = frozenset((213,))
ENHANCED_AVAILABILITY_MARKER = "(Enhanced) - Availability"

EVENT_MANUAL = 0
EVENT_LEVEL = 1
EVENT_NPC_RACE_KILL = 3
EVENT_TASK_COMPLETE = 4
EVENT_ZONE_ENTER = 5
EVENT_OWN_ITEM = 7
EVENT_SKILL_VALUE = 9
EVENT_ALTERNATE_ADVANCEMENT = 10
EVENT_ACHIEVEMENT_COMPLETE = 11
EVENT_NPC_NAME_KILL = 12
EVENT_SKILL_CAP = 13

PROGRESS_INCREMENT = 0
PROGRESS_SET = 2
PROGRESS_BOOLEAN = 3

BEHAVIOR_REQUIRED = 0
BEHAVIOR_OPTIONAL = 1


# These mappings are tied to the reviewed RoF2/DoN resource snapshot. Shape
# digests make the broad raid and Hunter mappings fail closed if a later client
# changes an ID, description, sequence, or required count.
REVIEWED_RAID_SHAPE_DIGESTS: dict[int, str] = {
    103240: "60e4921234d6a98447b6a6ad5fd8f69c2937b5006025e3f8f077186b586bf5c5",
    103940: "085fdab71ad53b51a11118958c3b7eee2011aa6cda71d0cf614859ff5ee37b1d",
    106440: "0ad5282f4cb8e3275aed482d9d58573716ceeddef4f5dc56ea4cbd9cd2faa712",
    107140: "9783c74275d49394f0c21c02deed4035d52f6fd99207bd87169578a6d86f8da6",
    107240: "218e7dd8847ca03503b4c7a2953880f30883946136f50e3cdc53f9bd03398bd7",
    107340: "06b65626f3cc8c87297a7206d90e5761c0685d6e8a1f03933eb3094819c33245",
    118640: "ca0b940728fb29ad524b9930bbc0370b2be6293af3d02c35147636770894ff57",
    127840: "2f670a0a7b3430c1cdc73fb9e0bf3920790bea89baf61a9ae88805039fda71ac",
    208640: "17ec56e4b9d8151ccb837d68b4db0e14817855ef778f6909be70ffcf04c8bea6",
    208940: "e836a9732691413465064cbf11f59a3504936734fee6e698dad2fcd0b34d4668",
    209140: "6bd4ce3f5e8e9e00aeaa26eedd4ce65bef94d507b2879cc2a49e292ab5869972",
    209440: "bd96996ae0e02c1ba75043bd4d2144ee421e313bfa2d3d7cf61bf99771c19851",
    209640: "9216352308579a6bff5084364296bc613146f42b330306f94585b1c7ed326ff9",
    210240: "1f75182ab1961b158abd5131417225c5de080179eedb271be7264184e322fd65",
    210340: "2697a9ee69f6092488f0de4c1ecaf301cb0da34aa450075d76239181fad4a92a",
    210540: "37ab5d3adc5f1046c12fa60a51186e54c33d362b60fd81fcee7059b54ae505a5",
    210840: "f63fdd8459588879d37042675760109812403d4c0d03640b4382b87ac2e79c55",
    210940: "9b3a4a2b4e934f34b5cc5f80192662f3bc26c22c180ee9ab9e8de835ca7a2949",
    227740: "6ecfcddf9b076d4a1c4a8db95ec94e677bc388cf924a37fa21dba0562c22ef37",
    311040: "a2b1c7bd25d1c4c79376548bd626754bbe4cfe8d319cf6968b126533e860835a",
    311240: "f7f3a3ba713f89df17bc1c8b198cd48b7f428d9d3b3c33d950b653fee8ecf7b4",
    311340: "88707f715e28b4701fa4209de2410dfc6c803c2bcccb31845352644b42093ff3",
    311440: "1eb3306fc26a9e35eee149513675cb030a8d8a5ab48be19644453e695af1bd70",
    311740: "614bc0190976f9f5a993486cdc0edc810ff736de4c6ba5e6e828dd32414f0011",
    311840: "4f6431a06513dd4e11fac96b8ba39b25d49cdf829b385f8e241b65e9e3310ade",
    311940: "8eadf630cfb327e5df19383c8587c12dee60ac775cbd2ea9b405170f524e37dd",
    312040: "cb49986610b809d6d9c9f29aeb991b043969d48637074be014a68eec0a7e6455",
    312340: "7c70423c026b3cc6c63d0ab8e0b16c00d0ab9218aad53364ae2738be7c0ff178",
    312440: "553a132b781726f3fd31d1e7853717063e0a32afa0e0d0716312d1671d729bd5",
    312740: "7196a7cd35f518294c0d383fef808214b35f343c307e2b719d0c7dfa833d66ad",
    312940: "c85dcb2f5adeaa9f5f574cdf3bb75506dbb1cb991a28371bd62c8b70862dae2c",
    352640: "6cfa1b13471f7e227d9b85ed84a470dc7c919578fb6e03b7d4bce2ff499bf1ba",
    380240: "23122cc7ed4c1c1105453b9867748b9f70c9dcda3443199c6be5e568b7cca48c",
    415340: "7e70b933ea4af12a396bb7afbe3e626099cade178aa6bfedbce340092f77fd29",
    415440: "1082660841f82be40f0423433cf9746a87d4b2c490d72c71d64a43279817b009",
    415840: "791fdfe71fa226b83af60fbdfaf96d7becd44d8458a65530027c30de7e9b3879",
    415940: "6008d325698f3f16e37e25e9ef4ec0f77b9f6c162d9b17e7b51a67adf614eaf0",
    416040: "6cb1936d38ec1d0df784e8da7147dccbca36cc47a9ed054761ab0cf55e3ef338",
    416240: "5ff22dc28fc2188a4a9336a309a772c8a0615d7c26026c76850f5833dc55aa7e",
    416340: "696314bac93f94e18291c6952e575f08bd59868d9ab27829626a7a80d40024b5",
    416440: "117366838381b51c1211d26de0b3bb4c4505cd27f804b1eac47f3c0904a6c2f1",
    417240: "5ea7707410aae2d5ce54463338969bd670946e0e69a62ce8d92d8bb4ac1e2a69",
    417640: "3c7118dba89d66084f9d49babe0180a5586b73de8f97c4bc96be3a313fee1978",
    417940: "ec2bb99b7bf971ee29f50c00705d912e01d77b5782aff7bb9cdac14094aededd",
    520040: "79b34d90643ebd833c310a67e4708e899f466439dbdd1e4208f7e6ff0d967c25",
    520140: "d1694e709baa665d1fc86ddb47361bb2f38e469f2129ec5d30fb70d891b0fe4c",
    520440: "4a3cb5e5fba3dee614187301be76656bda0441aed124d666e3721006e1f206be",
    520540: "98b1f6d7e8aeff53fc71248815660ce7012ddee1fcacf576342c487432610c14",
    520640: "7617def79a9c8e3b0fc962c8f5fc6e18119d6d0c125aed47a1725dc270a1921d",
    520740: "9968af20f31baa2212effe34d51c7ad95f3a2e3739618d69aea99d3fdc739918",
    520840: "c8631009c28804f6560c4cd4e5e7341007aca0ff27e6008b4bcdba1d01e3f84b",
    520940: "88f0fa337ef2f1ef290ab7725bef65a82ba26baa3297a05597cb4a76d81ec329",
    521040: "cf5480a0e94c09f2d8ae3bcaf1dd4ec46514d05bad69934125dd3e0e788cd869",
    521140: "ec2cfbd6b816def4e634a32d10465b318158bc28f5b03b33d44b0698c2e40660",
    521240: "649d0307c02d5cb1873c541df4b9c7455a35d7f47318e4d8c6b0944cf221f31f",
    521440: "33223ec5859b3fb4e3a21ad1f3f317f8beb9030997b5715f9b03c73c6b2a7fb2",
    521540: "37c1bff093018fbbf4a3b81e95a9e39072082a11ec35dd8ebeaccb96464f3d22",
    521640: "184310bc67569f82dd97942f6316831125ee6f7d6f6bad609d6865d9e54a001e",
    521740: "c4542def7f1a9f6dd209a7298a2f2fb1a50304c2387e710cb6dd9d3587086ce7",
    521840: "b2c6302cd6ebb51af28843e5e43dd68a499bc64d83096c7c7a69410cd3eceb28",
    522040: "a839acd489c03edb7147288e62b7a1f257767f2e00df92427d570618701e34ee",
    522140: "81aab3b73e9eb2cbbe857b229303f815c376b2bbfcf9a0ba68574f0cd61ae316",
    522240: "2c7936b42b9a7ee2c0946fcc8397ffc3fcdc164c5e3e60c8f4a4dfb768f6f5d5",
    573640: "d43c9e75e1f7b069fc67b2e19c670f222a1875de67f28701f47ed829cb6a3e05",
    622740: "6d2d749c68757f76bfd838a8240945dbe5a15b7a85185ec77199036b876e809d",
    622840: "7b75edc6029d0fdb415772e244755732be89d4a2b6b77a2800f8a4f477d7ecca",
}

REVIEWED_HUNTER_SHAPE_DIGESTS: dict[int, str] = {
    139280: "aae85fa7de88bff8f44943166c9f869223498afb9860bafc9731b5f4616d00a2",
    139380: "162f647a2e454113c239a1e0f8b9c4b4677310870a3fd29e8cb9ee7c17c2f057",
    250880: "cf4e4c1fd0133a33fd80278ccdb5749bb0287d648e90eb937770340c1a9e7e19",
    258180: "42245ced143e2e4f787aa9b5a4fb6753957a2825d1f7494b41568d5e5f5e6aee",
    520080: "905758afa103eb9387ae7c9e371c50bb0a3014d98004ae855116aa054b40b828",
    808500: "5475847d5aca416c6514d717b0b2d5089f8f1ec3749933a422558f89292b7fff",
    908300: "4226498be4bb681cf83e33f9bc302823b858323b0e978ab763873571e4245a3c",
    908600: "c8efe14e127959f453dfdf416269f570e90b3887ce985cf378e6a13c25f7376e",
    908700: "0e6e3786251c63b982457b40b43923ddc2274061f3ac7d0838da689705833d99",
}

REVIEWED_RAID_SCRIPT_COMPONENTS = frozenset(
    {
        (521140, 1, 5000284), (521140, 1, 5000285),
        (521140, 1, 5000286),
        (573640, 1, 5000225), (573640, 1, 5000358),
        (573640, 1, 5000359), (573640, 1, 5000360),
        (573640, 1, 5000361), (573640, 1, 5000362),
        (722940, 1, 7000159), (722940, 1, 7000161),
        (723040, 1, 7000168), (723040, 1, 7000169),
        (723140, 1, 7000172), (723240, 1, 7000163),
        (723340, 1, 7000166),
    }
)

REVIEWED_RAID_UNAVAILABLE_COMPONENTS = frozenset(
    {
        (200071, 1, 200098), (153940, 1, 1001516),
        (352640, 1, 3000338), (415440, 1, 4000305),
        (415840, 1, 4000324), (415840, 1, 4000326),
        (415840, 1, 4000334), (415840, 1, 4000335),
        (520440, 1, 5000265),
        (722940, 1, 7000160), (723040, 1, 7000167),
        (723140, 1, 7000170), (723140, 1, 7000171),
        (723240, 1, 7000162), (723240, 1, 7000164),
        (723340, 1, 7000165),
    }
)

REVIEWED_RAID_ZONE_OVERRIDES = {380240: 128, 573640: 223}
REVIEWED_NPC_NAME_OVERRIDES: dict[Tuple[int, int, int], str] = {
    (107140, 1, 1001518): "the Hand of Veeshan",
    (210340, 1, 2001361): "Queen Velazul Di`zok",
    (312440, 1, 3000234): "Eashen of Sky",
    (352640, 1, 3000339): "Bristlebane the King of Thieves",
    (521040, 1, 5000271): "Ston`Ruak, Ancient of Trees",
    (520080, 1, 5000031): "War Chieftan Dorwikak",
}
REVIEWED_NPC_NAME_TARGETS: dict[
    Tuple[int, int, int], Tuple[str, ...]
] = {
    (521640, 2, 5000334): (
        "a hungry anglerfish",
        "a stringy anglerfish",
        "a toughened anglerfish",
    ),
    (521640, 2, 5000335): (
        "a dark anglerfish",
        "a wicked anglerfish",
        "a foul anglerfish",
    ),
    (521640, 2, 5000336): (
        "a superior anglerfish",
        "a prime anglerfish",
        "a prismatic anglerfish",
    ),
    (521640, 1, 5000337): (
        "a king anglerfish",
        "a master anglerfish",
        "a supreme anglerfish",
    ),
    (521640, 2, 5000337): (
        "a king anglerfish",
        "a master anglerfish",
        "a supreme anglerfish",
    ),
}

REVIEWED_HUNTER_DEFAULT_ZONES = {
    139280: 34,
    250880: 108,
    258180: 81,
    520080: 200,
}
REVIEWED_HUNTER_COMPONENT_ZONES: dict[Tuple[int, int, int], int] = {
    (139380, 1, 1001169): 35,
    (139380, 1, 1001171): 37,
    (139380, 1, 1001170): 37,
    (808500, 1, 80850001): 286,
    (808500, 1, 80850002): 288,
    (808500, 1, 80850003): 288,
    (808500, 1, 80850004): 285,
    (808500, 1, 80850005): 285,
    (808500, 1, 80850006): 287,
    (808500, 1, 80850007): 287,
    (808500, 1, 80850008): 286,
    (808500, 1, 80850009): 288,
    (808500, 1, 80850010): 286,
    (808500, 1, 80850011): 287,
    (808500, 1, 80850012): 285,
    (908300, 1, 90830001): 318,
    (908300, 1, 90830002): 318,
    (908300, 1, 90830003): 320,
    (908300, 1, 90830004): 319,
    (908300, 1, 90830005): 320,
    (908300, 1, 90830006): 319,
    (908600, 1, 90860001): 328,
    (908600, 1, 90860002): 329,
    (908600, 1, 90860003): 330,
    (908600, 1, 90860004): 328,
    (908600, 1, 90860005): 330,
    (908600, 1, 90860006): 329,
    (908700, 1, 90870001): 333,
    (908700, 1, 90870002): 332,
    (908700, 1, 90870003): 332,
    (908700, 1, 90870004): 333,
    (908700, 1, 90870005): 331,
    (908700, 1, 90870006): 331,
}

REVIEWED_HUNTER_UNAVAILABLE_COMPONENTS = frozenset(
    {
        (139380, 1, 1001172),
        (250880, 1, 2001247),
    }
)

REVIEWED_OWN_ITEM_TARGETS: dict[
    Tuple[int, int, int], Tuple[int, ...]
] = {
    (90007, 1, 3014): (12708,),
    (90022, 2, 3036): (11173,),
    (415705, 1, 415705): (28771,),
    (520205, 1, 500036): (16257, 16255),
    (520205, 2, 500037): (16249,),
    (520205, 2, 500038): (16250,),
    (520205, 2, 500039): (16251,),
    (520205, 2, 500040): (16252,),
    (520205, 2, 500041): (32800,),
    (520205, 2, 500042): (16254,),
    (520205, 2, 500043): (16256,),
    (900100, 1, 900100): (52415,),
    (900100, 2, 900101): (52400,),
    (900100, 2, 900102): (52401,),
    (900100, 2, 900103): (52402,),
    (900100, 2, 900104): (52403,),
    (900100, 2, 900105): (52404,),
    (900100, 2, 900106): (52405,),
    (900100, 2, 900107): (52406,),
    (900110, 1, 900110): (52413,),
    (500980300, 1, 59803000): (41000,),
}

REVIEWED_TASK_TARGETS: dict[
    Tuple[int, int, int], Tuple[int, ...]
] = {
    (600000, 1, 600000): (8799,),
    (500980500, 1, 59805000): (402008,),
    (500980500, 1, 59805001): (402010,),
    (500980500, 1, 59805002): (402011,),
    (500980500, 1, 59805003): (402012,),
    (500980500, 1, 59805004): (402013,),
    (500980500, 1, 59805005): (402014,),
    (500980500, 1, 59805006): (402015,),
    (500980500, 1, 59805007): (402017,),
    (500980500, 1, 59805008): (402018,),
    (500980610, 2, 59806102): (4827,),
    (500980610, 2, 59806103): (4828,),
    (500980610, 2, 59806104): (4829,),
    (500980610, 2, 59806106): (5015,),
    (500980620, 2, 59806202): (4830,),
    (500980620, 2, 59806203): (4831,),
    (500980620, 2, 59806204): (4832,),
    (500980620, 2, 59806206): (5555,),
    (500980620, 2, 59806207): (5580, 5581),
    (500980630, 2, 59806302): (4833,),
    (500980630, 2, 59806303): (4834,),
    (500980630, 2, 59806304): (4835,),
    (500980630, 2, 59806306): (4771,),
    (500980630, 2, 59806307): (5053, 5054),
    (500980630, 2, 59806308): (5582, 5583),
    (500980640, 2, 59806402): (5584, 5585),
    (500980640, 2, 59806403): (4952, 4953),
    (500980640, 2, 59806405): (5506, 5507),
    (500980650, 2, 59806502): (4986,),
    (500980650, 2, 59806503): (5579,),
    (500980650, 2, 59806505): (5508, 5509),
    (500980660, 2, 59806602): (5044,),
    (500980660, 2, 59806603): (5045,),
    (500980660, 2, 59806604): (5046,),
    (500980660, 2, 59806606): (401,),
    (500980670, 2, 59806702): (5047,),
    (500980670, 2, 59806703): (5048,),
    (500980670, 2, 59806704): (5049,),
    (500980670, 2, 59806706): (4785,),
    (500980670, 2, 59806707): (5580, 5581),
    (500980680, 2, 59806802): (5050,),
    (500980680, 2, 59806803): (5051,),
    (500980680, 2, 59806804): (5052,),
    (500980680, 2, 59806806): (4799,),
    (500980680, 2, 59806807): (5053, 5054),
    (500980680, 2, 59806808): (5582, 5583),
    (500980690, 2, 59806902): (5584, 5585),
    (500980690, 2, 59806903): (4952, 4953),
    (500980690, 2, 59806905): (5506, 5507),
    (500980700, 2, 59807002): (4991,),
    (500980700, 2, 59807003): (5578,),
    (500980700, 2, 59807005): (5508, 5509),
}

REVIEWED_DEPENDENCY_TARGETS: dict[
    Tuple[int, int, int], Tuple[int, ...]
] = {
    (100060, 1, 1001490): (103940,),
    (100095, 1, 1001022): (103980,),
    (100110, 1, 1001052): (101880,),
    (100110, 1, 1001056): (104880,),
    (600080, 1, 6000001): (127880,),
    (500980600, 1, 59806000): (500980605,),
    (500980600, 1, 59806001): (500980610, 500980660),
    (500980600, 1, 59806002): (500980620, 500980670),
    (500980600, 1, 59806003): (500980630, 500980680),
    (500980600, 1, 59806004): (500980640, 500980690),
    (500980600, 1, 59806005): (500980650, 500980700),
}

REVIEWED_PRESENTATION_COMPONENTS = frozenset(
    {
        (500980610, 2, 59806101), (500980610, 2, 59806105),
        (500980620, 2, 59806201), (500980620, 2, 59806205),
        (500980630, 2, 59806301), (500980630, 2, 59806305),
        (500980640, 2, 59806401), (500980640, 2, 59806404),
        (500980650, 2, 59806501), (500980650, 2, 59806504),
        (500980660, 2, 59806601), (500980660, 2, 59806605),
        (500980670, 2, 59806701), (500980670, 2, 59806705),
        (500980680, 2, 59806801), (500980680, 2, 59806805),
        (500980690, 2, 59806901), (500980690, 2, 59806904),
        (500980700, 2, 59807001), (500980700, 2, 59807004),
    }
)

REVIEWED_PROGRESSION_UNAVAILABLE_IDS = frozenset(
    {
        12007, 12009, 12010, 12013, 12014, 12016, 12018, 12019,
        12022, 12030, 12037, 12038, 12046, 12047, 12048, 12062,
        12081, 12091, 12108, 12112, 12120, 12129,
    }
)
REVIEWED_PROGRESSION_SCRIPT_IDS = frozenset(
    set(range(12000, 12144))
    - {12080}
    - REVIEWED_PROGRESSION_UNAVAILABLE_IDS
)

REVIEWED_PROGRESSION_PROFILE_IDS = frozenset(
    REVIEWED_PROGRESSION_SCRIPT_IDS
    | {
        500980300,
        500980400,
        500980500,
        500980530,
        500980550,
        500980600,
        500980605,
        500980610,
        500980620,
        500980630,
        500980640,
        500980650,
        500980660,
        500980670,
        500980680,
        500980690,
        500980700,
        500990020,
    }
)

REVIEWED_MANUAL_SCRIPT_COMPONENTS = frozenset(
    {
        (500980400, 1, 59804000),
        *((500980400, 2, value) for value in range(59804001, 59804006)),
        (500980500, 1, 59805014),
        *((500980500, 1, value) for value in range(59805009, 59805014)),
        (500980530, 1, 59805300),
        *((500980530, 2, value) for value in range(59805301, 59805315)),
        (500980550, 1, 59805500),
        *((500980550, 2, value) for value in range(59805501, 59805511)),
        *((500990020, 1, value) for value in range(59900200, 59900206)),
    }
)

REVIEWED_DON_SCRIPT_COMPONENTS = frozenset(
    {
        (500980605, 1, 59806050),
        (500980605, 2, 59806051),
        (500980605, 2, 59806052),
        (500980610, 1, 59806100),
        (500980610, 1, 59806107),
        (500980620, 1, 59806200),
        (500980620, 1, 59806208),
        (500980630, 1, 59806300),
        (500980630, 1, 59806309),
        (500980640, 1, 59806400),
        (500980640, 1, 59806406),
        (500980650, 1, 59806500),
        (500980650, 1, 59806506),
        (500980660, 1, 59806600),
        (500980660, 1, 59806607),
        (500980670, 1, 59806700),
        (500980670, 1, 59806708),
        (500980680, 1, 59806800),
        (500980680, 1, 59806809),
        (500980690, 1, 59806900),
        (500980690, 1, 59806906),
        (500980700, 1, 59807000),
        (500980700, 1, 59807006),
    }
)

REVIEWED_KEY_AND_MPG_SCRIPT_COMPONENTS = frozenset(
    {
        (90020, 1, 3033),
        (90022, 1, 3035),
        (90023, 1, 3037),
        (90028, 1, 3042),
        *((900110, 2, value) for value in range(900111, 900117)),
    }
)

REVIEWED_SCRIPT_COMPONENTS = frozenset(
    REVIEWED_RAID_SCRIPT_COMPONENTS
    | REVIEWED_MANUAL_SCRIPT_COMPONENTS
    | REVIEWED_DON_SCRIPT_COMPONENTS
    | REVIEWED_KEY_AND_MPG_SCRIPT_COMPONENTS
    | {
        (achievement_id, 1, achievement_id)
        for achievement_id in REVIEWED_PROGRESSION_SCRIPT_IDS
    }
)

REVIEWED_PARTIAL_UNAVAILABLE_COMPONENTS = frozenset(
    {
        (100105, 2, 1001041),
        (800050, 2, 270588),
        (500080, 2, 5000367),
    }
)

REVIEWED_TRADESKILL_UNAVAILABLE_IDS = frozenset(
    {
        base + offset
        for base in (
            556300, 557300, 558300, 559300, 560300,
            561300, 563300, 564300, 565300, 568300, 569300,
        )
        for offset in (10, 20, 30, 40, 50)
    }
    | {555500, 590100, 590101, 590110, 590111}
)

REVIEWED_UNAVAILABLE_DEFINITION_GROUPS: Tuple[
    Tuple[str, frozenset[int]], ...
] = (
    (
        "housing, guild-hall, wedding, and special-event zones are absent "
        "from the RoF2/DoN server profile",
        frozenset(
            {
                100038400, 100040800, 100049300, 100049400, 100049500,
                100071400, 100071500, 100071600, 100071700, 100071800,
                100071900, 100072300, 100073700, 100073800, 100075100,
                100076600, 100077400,
            }
        ),
    ),
    (
        "revamped or enhanced Hunter zone population is not available in this server profile",
        frozenset({138480, 151880, 153980, 154880}),
    ),
    (
        "The Plane of War and its progression content are not available in this server profile",
        frozenset(
            {
                90034, 200071, 521300, 521301, 521302, 521303,
                521304, 521305, 521306, 521307, 521308, 521309, 521310,
            }
        ),
    ),
    (
        "the required zone or instance is not available in this server profile",
        frozenset({899800}),
    ),
    (
        "the milestone exceeds the Dragons of Norrath AA progression profile",
        frozenset({103250, 103500}),
    ),
    (
        "the tutorial belongs to post-DoN Gloomingdeep content",
        frozenset({600050}),
    ),
    (
        "the achievement belongs to later progression content and "
        "has no DoN-era completion source",
        frozenset({500980100, 500980200}),
    ),
    (
        "the skill target or Artisan quest belongs to post-DoN tradeskill progression",
        REVIEWED_TRADESKILL_UNAVAILABLE_IDS,
    ),
    (
        "no canonical durable quest completion source was found in the quests workspace",
        frozenset({90003, 90033, 90035}),
    ),
    (
        "a required named NPC is absent from the audited PEQ spawn data "
        "(Erg Bluntbruiser or Blood-Thirsty Racnar)",
        frozenset({139380, 250880}),
    ),
    (
        "a required raid target is absent from the audited PEQ spawn data",
        frozenset({153940, 352640, 415440, 415840, 520440}),
    ),
    (
        "one or more required LDoN raid variants have no authoritative "
        "encounter implementation or completion hook",
        frozenset({722940, 723040, 723140, 723240, 723340}),
    ),
    (
        "no authoritative success hook was found in the quests workspace",
        frozenset(REVIEWED_PROGRESSION_UNAVAILABLE_IDS - {12016}),
    ),
    (
        "the owning quest NPC is not spawned in the audited PEQ content",
        frozenset({12016}),
    ),
)

REVIEWED_UNAVAILABLE_ACHIEVEMENT_IDS = frozenset(
    achievement_id
    for _reason, achievement_ids in REVIEWED_UNAVAILABLE_DEFINITION_GROUPS
    for achievement_id in achievement_ids
)

REVIEWED_INVESTIGATION_ACHIEVEMENT_IDS = frozenset(
    REVIEWED_PROGRESSION_UNAVAILABLE_IDS
    | {
        138480, 139380, 151880, 153940, 153980, 154880, 250880,
        352640, 415440, 415840, 520440,
        722940, 723040, 723140, 723240, 723340,
    }
)

REVIEWED_FORCED_DISABLED_ACHIEVEMENT_IDS = frozenset(
    REVIEWED_UNAVAILABLE_ACHIEVEMENT_IDS
    - REVIEWED_INVESTIGATION_ACHIEVEMENT_IDS
)


# Exact Slayer vocabulary mapped to stable RoF2 Race IDs from common/races.h.
# Unknown terms are rejected instead of guessed. Related model races are
# included only when the client term names the shared creature family.
SLAYER_RACE_TERM_IDS: dict[str, Tuple[int, ...]] = {
    "Akhevans": (230, 722),
    "Alarans": (695,),
    "Alligators": (91, 479),
    "Amygdalans": (99, 663),
    "Animated Armors": (323,),
    "Animated Hands": (166,),
    "Apexus": (637,),
    "Apes": (41, 560),
    "Armadillos": (87,),
    "Aviaks": (13, 558),
    "Barbarians": (2, 90),
    "Barrels": (377,),
    "Basilisks": (436,),
    "Banshees": (250, 487, 488),
    "Bats": (34, 260, 416),
    "Bazus": (409,),
    "Bears": (43, 305, 480),
    "Beetles": (22, 207, 559, 716),
    "Bellikos": (638,),
    "Bixies": (79, 520),
    "Blind Dreamers": (669,),
    "Blood Ravens": (279,),
    "Boars": (319, 321),
    "Bones": (383,),
    "Book Dervishes": (660,),
    "Book Minion": (660,),
    "Boxes": (376,),
    "Brontotheriums": (169,),
    "Brownies": (15, 568),
    "Braxi": (688,),
    "Bubonians": (268, 269),
    "Burynai": (144, 602),
    "Cats": (713,),
    "Centaurs": (16, 521),
    "Chests": (378, 589, 590),
    "Chimeras": (412, 582),
    "Chokidais": (356, 357),
    "Cliknar Soldiers": (643,),
    "Cliknars": (642, 643, 644),
    "Clocks": (665,),
    "Cockatrices": (96,),
    "Coffins": (382, 592),
    "Coldain": (183, 645, 646),
    "Coldains": (183, 645, 646),
    "Corathus Beasts": (459,),
    "Crabs": (302,),
    "Cragbeasts": (390,),
    "Crocodiles": (259,),
    "Crystal Spheres": (616,),
    "Crystalskins": (641, 647),
    "Cubes": (31, 712),
    "Dark Elves": (6, 77),
    "Dervishes": (100, 170, 372, 431, 704, 726, 727),
    "Devourers": (159, 286),
    "Djinns": (126,),
    "Drachnids": (57, 461),
    "Dracoliches": (604,),
    "Dragons": (
        49,
        122,
        165,
        184,
        192,
        195,
        196,
        198,
        304,
        435,
        437,
        438,
        452,
        530,
        531,
        569,
    ),
    "Dragorns": (413,),
    "Drakes": (89, 430, 432),
    "Drakkin": (522,),
    "Drixies": (113,),
    "Drogmores": (348,),
    "Drolvargs": (133,),
    "Dryads": (243,),
    "Dwarves": (8, 94),
    "Efreetis": (101, 320),
    "Elddar Elves": (489,),
    "Elephants": (107, 528),
    "Elementals": (
        75,
        84,
        120,
        209,
        210,
        211,
        212,
        475,
        476,
        477,
        478,
    ),
    "Enchanted Armors": (175,),
    "Erudites": (3, 78, 678),
    "Evil Eyes": (21, 375, 469),
    "Eyes": (21, 108, 375, 469),
    "Fairies": (25, 473),
    "Fauns": (182,),
    "Fay Drakes": (154,),
    "Ferans": (410,),
    "Fiends": (218, 253, 300),
    "Fishes": (24, 148),
    "Flies": (245,),
    "Frogloks": (26, 27),
    "Frogs": (343, 603),
    "Fungal Fiends": (218,),
    "Gargoyles": (29, 280, 464),
    "Gelatinous Cubes": (31, 712),
    "Gelidrans": (417,),
    "Genari": (648,),
    "Geonids": (178,),
    "Ghosts": (32, 117, 118, 334, 588),
    "Ghouls": (33, 571),
    "Giants": (18, 140, 188, 189, 306, 307, 308, 309, 310, 311, 312, 453, 523),
    "Gigyns": (649,),
    "Gingerbread Men": (666,),
    "Gnolls": (39, 524, 617),
    "Gnomes": (12,),
    "Gnomeworks": (457,),
    "Goblins": (40, 59, 137, 369, 433),
    "Golems": (17, 160, 164, 248, 362, 374, 491),
    "Goos": (145, 547, 548, 549),
    "Gorals": (687,),
    "Gorgons": (121,),
    "Gorillas": (41, 560),
    "Greken": (650, 651),
    "Grekens": (650, 651),
    "Grendlaens": (701,),
    "Griffennes": (47, 525),
    "Griffins": (47, 525),
    "Griffons": (47, 525),
    "Grimlings": (202,),
    "Guktans": (330, 349, 350, 371),
    "Hadal": (698,),
    "Hags": (185,),
    "Half Elves": (7,),
    "Halflings": (11, 81),
    "Harpies": (111, 527),
    "High Elves": (5,),
    "Holgresh": (168, 715),
    "Horses": (216, 492, 518),
    "Hraquis": (261,),
    "Humans": (1, 67, 71, 566),
    "Hydra Crystals": (615,),
    "Hynids": (388,),
    "Iksars": (128, 139),
    "Imps": (46,),
    "Insects": (370,),
    "Kangons": (689,),
    "Kedge": (103, 561),
    "Kerrans": (23, 562),
    "Kirins": (434, 583),
    "Kobolds": (48, 455),
    "Kodiaks": (43, 305, 480),
    "Krakens": (315,),
    "Kylong Iksars of Veksar": (353, 354, 355),
    "Leeches": (104,),
    "Lepertoloths": (267,),
    "Lightcrawlers": (223,),
    "Lions": (50,),
    "Lizard Men": (51,),
    "Luggalds": (345, 346, 347),
    "Malarian": (265,),
    "Malarians": (265,),
    "Mammoths": (107, 528),
    "Manticores": (172,),
    "Mantraps": (573,),
    "Marionettes": (659,),
    "Mephits": (291, 292, 293, 294, 607),
    "Mermaids": (110,),
    "Mimics": (52,),
    "Minotaurs": (53, 420, 470, 574),
    "Molerats": (415,),
    "Mosquitoes": (134,),
    "Muddites": (608,),
    "Mummies": (368,),
    "Murkgliders": (414,),
    "Mystical Horses": (124, 125, 287, 493, 517, 519, 732),
    "Nightmare Goblins": (277,),
    "Nightmares": (287, 517, 519),
    "Nilborien": (317,),
    "Nymphs": (242,),
    "Ogres": (10, 93, 624),
    "Orcs": (54, 361, 366, 458),
    "Othmirs": (190,),
    "Otters": (190,),
    "Owlbears": (206,),
    "Pandas": (43, 305, 480),
    "Pegasus": (125, 493, 732),
    "Piranhas": (74,),
    "Pixies": (56,),
    "Pumas": (76, 439, 584),
    "Pyrilens": (411,),
    "Queens": (642,),
    "Rabbits": (176, 668),
    "Raptors": (163, 609),
    "Ratmen": (156, 718),
    "Rats": (36, 415),
    "Rhinoceros": (135,),
    "Rhinos": (135,),
    "Riftseekers": (411, 417),
    "Rockhoppers": (200,),
    "Rotdogs": (662,),
    "Sabertooths": (119,),
    "Sand Elves": (364,),
    "Sandmen": (664,),
    "Sarnaks": (131, 146, 155, 610),
    "Satyr": (529,),
    "Scarecrows": (82, 575),
    "Scaled Wolves": (481,),
    "Scarlet Cheetahs": (221,),
    "Scavengers": (700,),
    "Scorpions": (129, 149, 611),
    "Scrykin": (495,),
    "Sea Turtles": (194,),
    "Seahorses": (116,),
    "Selyrah": (686,),
    "Shades": (224, 373, 526, 576),
    "Shadows": (723,),
    "Sharks": (61,),
    "Shadels": (205,),
    "Shambling Mounds": (494,),
    "Shik'Nars": (199,),
    "Shiliskins": (467,),
    "Shissar": (217, 563),
    "Shriekers": (227,),
    "Sirens": (187, 564),
    "Skeletons": (60, 349, 367, 484, 606),
    "Skunks": (83,),
    "Snakes": (37, 468),
    "Sokokar": (618,),
    "Sokokars": (618,),
    "Sonic Wolves": (232,),
    "Sphinxes": (86, 565),
    "Spectres": (85, 174, 485),
    "Spiders": (38, 327, 440, 441, 450, 451),
    "Sporalis": (456,),
    "Spirits": (146, 147, 483),
    "Statues": (442, 448),
    "Stonemites": (391,),
    "Stonegrabbers": (220,),
    "Stormriders": (272,),
    "Succulents": (167,),
    "Sunflowers": (225,),
    "Swinetors": (696,),
    "Swordfishes": (105,),
    "Tables": (380,),
    "Tadpoles": (102,),
    "Taelosians": (403,),
    "Tegis": (215,),
    "Telmiras": (653,),
    "Tentacle Terrors": (68, 578),
    "Thought Horrors": (214,),
    "Tigers": (63,),
    "Tin Soldiers": (263,),
    "Topiary Lions": (661,),
    "Tormentors": (285,),
    "Totems": (173, 514),
    "Traps": (503, 506, 513),
    "Treants": (64, 244, 496),
    "Trolls": (9, 92, 331, 332, 333),
    "Tsetsians": (612,),
    "Turtles": (194,),
    "Tureptas": (389,),
    "Ulthorks": (191,),
    "Underbulks": (201,),
    "Unicorns": (124, 517, 519),
    "Vah Shir": (130, 238),
    "Vampires": (65, 98, 208, 219, 359, 360, 365, 497),
    "Vases": (379,),
    "Vegerogs": (258,),
    "Vine Maws": (717,),
    "Walruses": (177,),
    "Wasps": (109,),
    "Water Dragons": (165,),
    "Werebats": (416,),
    "Wereorcs": (579,),
    "Werewolves": (14, 241, 454),
    "Wetfang Minnows": (213,),
    "Webs": (515,),
    "Wisps": (69,),
    "Witherans": (465, 474),
    "Wolves": (42, 120, 171, 482, 483),
    "Wood Elves": (4,),
    "Workers": (644,),
    "Worgs": (580, 594),
    "Wraiths": (264, 313),
    "Wrulons": (314, 598),
    "Wurms": (158, 613),
    "Wyverns": (157, 581),
    "Yetis": (138,),
    "Zelniaks": (222,),
    "Zombies": (70, 344, 350, 471),
}

# Phrases that normal comma/and tokenization cannot express. Values remain exact
# RoF2 race IDs; phrases absent from this table are rejected.
SLAYER_RACE_DESCRIPTION_OVERRIDES: dict[str, Tuple[int, ...]] = {
    (
        "All creatures of discord: Aneuks, Discordlings, Girplans, Huvuls, "
        "Ikaavs, Ixts... See: Such Anguish"
    ): (393, 394, 395, 400, 418, 419),
    (
        "All creatures of discord: ...Kyvs, Lightning Warriors, Mastruqs... "
        "See Discord Sounds Out of Tune"
    ): (396, 402, 407),
    "Animated and Enchanted Armors.": (175, 323),
    "Guktans": (330, 349, 350, 371),
    "Guktans (Living, Skeletal, Ghostly, or Zombified)": (
        26,
        27,
        330,
        349,
        350,
        371,
    ),
    "The playable races.": (
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        10,
        11,
        12,
        67,
        71,
        77,
        78,
        81,
        90,
        92,
        93,
        94,
        128,
        130,
        139,
        238,
        330,
        349,
        350,
        371,
        331,
        332,
        333,
        522,
        566,
        624,
        678,
    ),
}

# Known stale ToB Slayer labels mapped to canonical achievement names. Unknown
# spellings remain unresolved.
SLAYER_DEPENDENCY_NAME_ALIASES: dict[str, str] = {
    "navies of luclin": "Natives of Luclin",
    "dark elf antonican, please": "Love Will Teir Them Apart",
    "50 shades repaid": "50 Shades...",
}


class ResourceError(ValueError):
    pass


@dataclass(frozen=True)
class ExpansionProfile:
    key: str
    resource_category_name: str
    level_cap: int
    aliases: Tuple[str, ...] = ()


# The category names are the top-level names used by the client resources.
# Level caps include only the unambiguous "General / Level" milestones; they
# are not treated as gameplay criteria for other definitions.
EXPANSION_PROFILES: Tuple[ExpansionProfile, ...] = (
    ExpansionProfile("classic", "EverQuest", 50, ("launch", "eq", "everquest")),
    ExpansionProfile("kunark", "Ruins of Kunark", 60, ("rok", "ruinsofkunark")),
    ExpansionProfile("velious", "Scars of Velious", 60, ("sov", "scarsofvelious")),
    ExpansionProfile("luclin", "Shadows of Luclin", 60, ("sol", "shadowsofluclin")),
    ExpansionProfile("pop", "Planes of Power", 65, ("planesofpower",)),
    ExpansionProfile("loy", "Legacy of Ykesha", 65, ("legacyofykesha",)),
    ExpansionProfile(
        "ldon",
        "Lost Dungeons of Norrath",
        65,
        ("lostdungeonsofnorrath",),
    ),
    ExpansionProfile("god", "Gates of Discord", 65, ("gatesofdiscord",)),
    ExpansionProfile("oow", "Omens of War", 70, ("omensofwar",)),
    ExpansionProfile("don", "Dragons of Norrath", 70, ("dragonsofnorrath",)),
    ExpansionProfile(
        "dodh",
        "Depths of Darkhollow",
        70,
        ("dod", "depthsofdarkhollow"),
    ),
    ExpansionProfile("por", "Prophecy of Ro", 70, ("prophecyofro",)),
    ExpansionProfile("tss", "The Serpent's Spine", 75, ("serpentsspine",)),
    ExpansionProfile("tbs", "The Buried Sea", 75, ("buriedsea",)),
    ExpansionProfile("sof", "Secrets of Faydwer", 80, ("secretsoffaydwer",)),
    ExpansionProfile("sod", "Seeds of Destruction", 85, ("seedsofdestruction",)),
    ExpansionProfile("uf", "Underfoot", 85, ("underfoot",)),
    ExpansionProfile("hot", "House of Thule", 90, ("houseofthule",)),
    ExpansionProfile("voa", "Veil of Alaris", 95, ("veilofalaris",)),
    ExpansionProfile("rof", "Rain of Fear", 100, ("rainoffear",)),
    ExpansionProfile(
        "cotf",
        "Call of the Forsaken",
        100,
        ("calloftheforsaken",),
    ),
    ExpansionProfile("tds", "The Darkened Sea", 105, ("darkenedsea",)),
    ExpansionProfile("tbm", "The Broken Mirror", 105, ("brokenmirror",)),
    ExpansionProfile("eok", "Empires of Kunark", 105, ("empiresofkunark",)),
    ExpansionProfile("ros", "Ring of Scale", 110, ("ringofscale",)),
    ExpansionProfile("tbl", "The Burning Lands", 110, ("burninglands",)),
    ExpansionProfile("tov", "Torment of Velious", 115, ("tormentofvelious",)),
    ExpansionProfile("cov", "Claws of Veeshan", 115, ("clawsofveeshan",)),
    ExpansionProfile("tol", "Terror of Luclin", 120, ("terrorofluclin",)),
    ExpansionProfile("nos", "Night of Shadows", 120, ("nightofshadows",)),
    ExpansionProfile("ls", "Laurion's Song", 125, ("laurionssong",)),
    ExpansionProfile("tob", "The Outer Brood", 125, ("outerbrood",)),
)


@dataclass(frozen=True)
class Category:
    category_id: int
    parent_id: int
    sequence: int
    name: str
    description: str
    icon: str


@dataclass(frozen=True)
class Achievement:
    achievement_id: int
    name: str
    description: str
    icon_id: int
    points: int
    has_reward: bool
    client_flag: int


@dataclass(frozen=True)
class CategoryAssociation:
    category_id: int
    sequence: int
    achievement_id: int


@dataclass(frozen=True)
class Component:
    achievement_id: int
    sequence: int
    component_type: int
    component_id: int
    name: str


@dataclass(frozen=True)
class ComponentCount:
    component_id: int
    required_count: int


@dataclass(frozen=True)
class LevelCriterion:
    achievement_id: int
    component_type: int
    component_sequence: int
    component_id: int
    level: int


@dataclass(frozen=True)
class EvaluationCriterion:
    achievement_id: int
    component_type: int
    component_sequence: int
    component_id: int
    event_type: int
    progress_mode: int
    behavior: int
    target_id: int
    target_id2: int
    target_value: int
    required_count: int = 1


@dataclass(frozen=True)
class ItemNameCriterion:
    achievement_id: int
    component_type: int
    component_sequence: int
    component_id: int
    behavior: int
    item_name: str
    required_class: int = 0
    required_count: int = 1


@dataclass(frozen=True)
class CriteriaReport:
    progression_level_candidates: int = 0
    progression_level_generated: int = 0
    progression_level_rejected: int = 0
    skill_cap_candidates: int = 0
    skill_cap_generated: int = 0
    skill_cap_rejected: int = 0
    item_definition_candidates: int = 0
    item_definition_selected: int = 0
    item_definition_rejected: int = 0
    item_name_mappings: int = 0
    traveler_candidates: int = 0
    traveler_generated: int = 0
    traveler_rejected: int = 0
    dependency_generated: int = 0
    dependency_superseded: int = 0
    dependency_ambiguous: int = 0
    dependency_self: int = 0
    dependency_rejected: int = 0
    tradeskill_candidates: int = 0
    tradeskill_generated: int = 0
    tradeskill_rejected: int = 0
    aa_spent_candidates: int = 0
    aa_spent_generated: int = 0
    aa_spent_rejected: int = 0
    npc_name_definitions: int = 0
    npc_name_unresolved_definitions: int = 0
    npc_name_generated: int = 0
    npc_name_rejected_components: int = 0
    npc_name_collision_components: int = 0
    slayer_candidates: int = 0
    slayer_direct_selected: int = 0
    slayer_meta_selected: int = 0
    slayer_generated: int = 0
    slayer_rejected: int = 0
    reviewed_source_components: int = 0
    reviewed_script_components: int = 0
    reviewed_presentation_components: int = 0
    reviewed_unavailable_components: int = 0


@dataclass(frozen=True)
class ResourceSet:
    categories: Tuple[Category, ...]
    achievements: Tuple[Achievement, ...]
    category_associations: Tuple[CategoryAssociation, ...]
    components: Tuple[Component, ...]
    component_counts: Tuple[ComponentCount, ...]
    warnings: Tuple[str, ...] = ()


@dataclass(frozen=True)
class EnableSelection:
    through_expansion: Optional[ExpansionProfile]
    max_level: Optional[int]
    expansion_achievement_ids: Tuple[int, ...]
    level_achievement_ids: Tuple[int, ...]
    level_criteria: Tuple[LevelCriterion, ...]
    max_tradeskill_skill: Optional[int] = None
    tradeskill_achievement_ids: Tuple[int, ...] = ()
    max_aa_spent: Optional[int] = None
    aa_spent_achievement_ids: Tuple[int, ...] = ()
    progression_level_achievement_ids: Tuple[int, ...] = ()
    progression_level_criteria: Tuple[LevelCriterion, ...] = ()
    skill_cap_achievement_ids: Tuple[int, ...] = ()
    item_achievement_ids: Tuple[int, ...] = ()
    item_name_criteria: Tuple[ItemNameCriterion, ...] = ()
    enable_slayer: bool = False
    slayer_achievement_ids: Tuple[int, ...] = ()
    slayer_rejections: Tuple[str, ...] = ()
    generated_criteria: Tuple[EvaluationCriterion, ...] = ()
    superseded_criteria: Tuple[EvaluationCriterion, ...] = ()
    reviewed_profile_achievement_ids: Tuple[int, ...] = ()
    forced_disabled_achievement_ids: Tuple[int, ...] = ()
    reviewed_source_component_keys: Tuple[Tuple[int, int, int], ...] = ()
    reviewed_script_component_keys: Tuple[Tuple[int, int, int], ...] = ()
    reviewed_presentation_component_keys: Tuple[Tuple[int, int, int], ...] = ()
    reviewed_unavailable_component_keys: Tuple[Tuple[int, int, int], ...] = ()
    reviewed_coverage_rejections: Tuple[str, ...] = ()
    criteria_report: CriteriaReport = field(default_factory=CriteriaReport)

    @property
    def achievement_ids(self) -> frozenset[int]:
        selected = frozenset(
            self.expansion_achievement_ids
            + self.level_achievement_ids
            + self.tradeskill_achievement_ids
            + self.aa_spent_achievement_ids
            + self.progression_level_achievement_ids
            + self.skill_cap_achievement_ids
            + self.item_achievement_ids
            + self.slayer_achievement_ids
            + self.reviewed_profile_achievement_ids
        )
        return selected - frozenset(self.forced_disabled_achievement_ids)


def _read_caret_rows(path: Path, field_count: int) -> Iterable[Tuple[int, Tuple[str, ...]]]:
    if not path.is_file():
        raise ResourceError(f"required resource file does not exist: {path}")

    try:
        source = path.open("r", encoding="utf-8-sig", errors="strict", newline=None)
    except OSError as error:
        raise ResourceError(f"unable to open {path}: {error}") from error

    with source:
        for line_number, source_line in enumerate(source, start=1):
            line = source_line.rstrip("\r\n")
            if not line:
                raise ResourceError(f"{path.name}:{line_number}: blank lines are not permitted")
            if not line.endswith("^"):
                raise ResourceError(
                    f"{path.name}:{line_number}: expected a trailing caret field delimiter"
                )

            fields = line.split("^")
            if fields[-1] != "" or len(fields) != field_count + 1:
                raise ResourceError(
                    f"{path.name}:{line_number}: expected {field_count} caret-delimited "
                    f"fields, found {len(fields) - 1}"
                )

            values = tuple(fields[:-1])
            for field_number, value in enumerate(values, start=1):
                if "\x00" in value:
                    raise ResourceError(
                        f"{path.name}:{line_number}: field {field_number} contains a NUL byte"
                    )
                try:
                    value.encode("utf-8", errors="strict")
                except UnicodeError as error:
                    raise ResourceError(
                        f"{path.name}:{line_number}: field {field_number} is not valid UTF-8"
                    ) from error

            yield line_number, values


def _uint(
    value: str,
    *,
    path: Path,
    line_number: int,
    field_name: str,
    maximum: int = UINT32_MAX,
    minimum: int = 0,
    empty_value: Optional[int] = None,
) -> int:
    if value == "" and empty_value is not None:
        return empty_value
    if not value or not value.isascii() or not value.isdecimal():
        raise ResourceError(
            f"{path.name}:{line_number}: {field_name} must be an unsigned decimal integer"
        )

    parsed = int(value, 10)
    if parsed < minimum or parsed > maximum:
        raise ResourceError(
            f"{path.name}:{line_number}: {field_name} must be between "
            f"{minimum} and {maximum}, found {parsed}"
        )
    return parsed


def _text(
    value: str,
    *,
    path: Path,
    line_number: int,
    field_name: str,
    maximum_length: Optional[int] = None,
) -> str:
    if maximum_length is not None and len(value) > maximum_length:
        raise ResourceError(
            f"{path.name}:{line_number}: {field_name} exceeds {maximum_length} characters"
        )
    return value


def _reject_duplicate(
    seen: set,
    key: object,
    *,
    path: Path,
    line_number: int,
    description: str,
) -> None:
    if key in seen:
        raise ResourceError(
            f"{path.name}:{line_number}: duplicate {description} key {key!r}"
        )
    seen.add(key)


def _load_categories(resource_directory: Path) -> Tuple[Category, ...]:
    path = resource_directory / "AchievementCategories.txt"
    result: List[Category] = []
    seen = set()
    for line_number, fields in _read_caret_rows(path, 6):
        parent_id, sequence, category_id, name, description, icon = fields
        row = Category(
            category_id=_uint(
                category_id,
                path=path,
                line_number=line_number,
                field_name="category_id",
                minimum=1,
            ),
            parent_id=_uint(
                parent_id,
                path=path,
                line_number=line_number,
                field_name="parent_category_id",
                empty_value=0,
            ),
            sequence=_uint(
                sequence,
                path=path,
                line_number=line_number,
                field_name="sequence",
            ),
            name=_text(
                name,
                path=path,
                line_number=line_number,
                field_name="name",
                maximum_length=255,
            ),
            description=_text(
                description,
                path=path,
                line_number=line_number,
                field_name="description",
            ),
            icon=_text(
                icon,
                path=path,
                line_number=line_number,
                field_name="icon",
                maximum_length=255,
            ),
        )
        _reject_duplicate(
            seen,
            row.category_id,
            path=path,
            line_number=line_number,
            description="category",
        )
        result.append(row)
    return tuple(result)


def _load_achievements(resource_directory: Path) -> Tuple[Achievement, ...]:
    path = resource_directory / "AchievementsClient.txt"
    result: List[Achievement] = []
    seen = set()
    for line_number, fields in _read_caret_rows(path, 7):
        (
            achievement_id,
            name,
            description,
            icon_id,
            points,
            has_reward,
            client_flag,
        ) = fields
        row = Achievement(
            achievement_id=_uint(
                achievement_id,
                path=path,
                line_number=line_number,
                field_name="achievement_id",
                minimum=1,
            ),
            name=_text(
                name,
                path=path,
                line_number=line_number,
                field_name="name",
                maximum_length=255,
            ),
            description=_text(
                description,
                path=path,
                line_number=line_number,
                field_name="description",
            ),
            icon_id=_uint(
                icon_id,
                path=path,
                line_number=line_number,
                field_name="icon_id",
            ),
            points=_uint(
                points,
                path=path,
                line_number=line_number,
                field_name="points",
            ),
            has_reward=bool(
                _uint(
                    has_reward,
                    path=path,
                    line_number=line_number,
                    field_name="has_reward",
                    maximum=UINT8_MAX,
                )
            ),
            client_flag=_uint(
                client_flag,
                path=path,
                line_number=line_number,
                field_name="client_flag",
                maximum=UINT8_MAX,
            ),
        )
        _reject_duplicate(
            seen,
            row.achievement_id,
            path=path,
            line_number=line_number,
            description="achievement",
        )
        result.append(row)
    return tuple(result)


def _load_category_associations(
    resource_directory: Path,
) -> Tuple[CategoryAssociation, ...]:
    path = resource_directory / "AchievementCategoryAssociationsClient.txt"
    result: List[CategoryAssociation] = []
    seen = set()
    for line_number, fields in _read_caret_rows(path, 3):
        category_id, sequence, achievement_id = fields
        row = CategoryAssociation(
            category_id=_uint(
                category_id,
                path=path,
                line_number=line_number,
                field_name="category_id",
                minimum=1,
            ),
            sequence=_uint(
                sequence,
                path=path,
                line_number=line_number,
                field_name="sequence",
            ),
            achievement_id=_uint(
                achievement_id,
                path=path,
                line_number=line_number,
                field_name="achievement_id",
                minimum=1,
            ),
        )
        key = (row.category_id, row.achievement_id)
        _reject_duplicate(
            seen,
            key,
            path=path,
            line_number=line_number,
            description="category association",
        )
        result.append(row)
    return tuple(result)


def _load_components(resource_directory: Path) -> Tuple[Component, ...]:
    path = resource_directory / "AchievementComponentsClient.txt"
    result: List[Component] = []
    seen = set()
    for line_number, fields in _read_caret_rows(path, 5):
        achievement_id, sequence, component_type, component_id, name = fields
        row = Component(
            achievement_id=_uint(
                achievement_id,
                path=path,
                line_number=line_number,
                field_name="achievement_id",
                minimum=1,
            ),
            sequence=_uint(
                sequence,
                path=path,
                line_number=line_number,
                field_name="sequence",
            ),
            component_type=_uint(
                component_type,
                path=path,
                line_number=line_number,
                field_name="component_type",
                maximum=3,
            ),
            component_id=_uint(
                component_id,
                path=path,
                line_number=line_number,
                field_name="component_id",
                minimum=1,
            ),
            name=_text(
                name,
                path=path,
                line_number=line_number,
                field_name="name",
            ),
        )
        key = (row.achievement_id, row.component_type, row.component_id)
        _reject_duplicate(
            seen,
            key,
            path=path,
            line_number=line_number,
            description="achievement component",
        )
        result.append(row)
    return tuple(result)


def _load_component_counts(resource_directory: Path) -> Tuple[ComponentCount, ...]:
    path = resource_directory / "AchievementAssociationsClient.txt"
    result: List[ComponentCount] = []
    seen = set()
    for line_number, fields in _read_caret_rows(path, 2):
        component_id, required_count = fields
        row = ComponentCount(
            component_id=_uint(
                component_id,
                path=path,
                line_number=line_number,
                field_name="component_id",
                minimum=1,
            ),
            required_count=_uint(
                required_count,
                path=path,
                line_number=line_number,
                field_name="required_count",
                maximum=UINT32_MAX,
                minimum=1,
            ),
        )
        _reject_duplicate(
            seen,
            row.component_id,
            path=path,
            line_number=line_number,
            description="component count",
        )
        result.append(row)
    return tuple(result)


def load_resources(
    resource_directory: Path, *, strict_references: bool = False
) -> ResourceSet:
    if not resource_directory.is_dir():
        raise ResourceError(f"resource directory does not exist: {resource_directory}")

    resources = ResourceSet(
        categories=_load_categories(resource_directory),
        achievements=_load_achievements(resource_directory),
        category_associations=_load_category_associations(resource_directory),
        components=_load_components(resource_directory),
        component_counts=_load_component_counts(resource_directory),
    )
    required_rows = (
        ("AchievementCategories.txt", resources.categories),
        ("AchievementsClient.txt", resources.achievements),
        (
            "AchievementCategoryAssociationsClient.txt",
            resources.category_associations,
        ),
        ("AchievementComponentsClient.txt", resources.components),
        ("AchievementAssociationsClient.txt", resources.component_counts),
    )
    for file_name, rows in required_rows:
        if not rows:
            raise ResourceError(f"{file_name}: expected at least one data row")

    warnings: List[str] = []

    category_ids = {row.category_id for row in resources.categories}
    achievement_ids = {row.achievement_id for row in resources.achievements}
    component_ids = {row.component_id for row in resources.components}

    missing_parents = sorted(
        {
            row.parent_id
            for row in resources.categories
            if row.parent_id != 0 and row.parent_id not in category_ids
        }
    )
    if missing_parents:
        warnings.append(
            "AchievementCategories.txt references missing parent categories: "
            + ", ".join(str(value) for value in missing_parents[:20])
        )

    missing_association_categories = sorted(
        {
            row.category_id
            for row in resources.category_associations
            if row.category_id not in category_ids
        }
    )
    if missing_association_categories:
        warnings.append(
            "AchievementCategoryAssociationsClient.txt references missing categories: "
            + ", ".join(str(value) for value in missing_association_categories[:20])
        )

    missing_association_achievements = sorted(
        {
            row.achievement_id
            for row in resources.category_associations
            if row.achievement_id not in achievement_ids
        }
    )
    if missing_association_achievements:
        warnings.append(
            "AchievementCategoryAssociationsClient.txt references missing achievements "
            f"({len(missing_association_achievements)} IDs; first values): "
            + ", ".join(str(value) for value in missing_association_achievements[:20])
        )

    missing_component_achievements = sorted(
        {
            row.achievement_id
            for row in resources.components
            if row.achievement_id not in achievement_ids
        }
    )
    if missing_component_achievements:
        warnings.append(
            "AchievementComponentsClient.txt references missing achievements "
            f"({len(missing_component_achievements)} IDs; first values): "
            + ", ".join(str(value) for value in missing_component_achievements[:20])
        )

    orphan_component_counts = sorted(
        {
            row.component_id
            for row in resources.component_counts
            if row.component_id not in component_ids
        }
    )
    if orphan_component_counts:
        warnings.append(
            "AchievementAssociationsClient.txt references missing components "
            f"({len(orphan_component_counts)} IDs; first values): "
            + ", ".join(str(value) for value in orphan_component_counts[:20])
        )

    if strict_references and warnings:
        raise ResourceError("; ".join(warnings))

    return ResourceSet(
        categories=resources.categories,
        achievements=resources.achievements,
        category_associations=resources.category_associations,
        components=resources.components,
        component_counts=resources.component_counts,
        warnings=tuple(warnings),
    )


def _normalized_name(value: str) -> str:
    return "".join(character for character in value.casefold() if character.isalnum())


def resolve_expansion(value: str) -> ExpansionProfile:
    normalized = _normalized_name(value)
    for profile in EXPANSION_PROFILES:
        names = (
            profile.key,
            profile.resource_category_name,
            *profile.aliases,
        )
        if normalized in {_normalized_name(name) for name in names}:
            return profile

    supported = ", ".join(profile.key for profile in EXPANSION_PROFILES)
    raise ResourceError(
        f"unknown expansion {value!r}; supported expansion names are: {supported}"
    )


def _category_descendants(
    resources: ResourceSet, root_category_ids: set[int]
) -> set[int]:
    category_ids = {row.category_id for row in resources.categories}
    descendants = set(root_category_ids)
    changed = True
    while changed:
        changed = False
        for category in resources.categories:
            if (
                category.category_id not in descendants
                and category.parent_id in descendants
            ):
                descendants.add(category.category_id)
                changed = True

    missing_roots = root_category_ids - category_ids
    if missing_roots:
        raise ResourceError(
            "cannot select expansion content because category roots are missing: "
            + ", ".join(str(value) for value in sorted(missing_roots))
        )
    return descendants


def canonicalize_npc_name(value: str) -> str:
    """Apply the server's NPC-name identity contract."""
    output: List[str] = []
    pending_space = False
    for character in value:
        if (
            "A" <= character <= "Z"
            or "a" <= character <= "z"
        ):
            if pending_space and output:
                output.append(" ")
            output.append(character.lower())
            pending_space = False
        elif character == "_" or character == " ":
            pending_space = bool(output)
        # Digits, punctuation, and non-ASCII codepoints are identity noise.

    return "".join(output)


def fnv1a_32(value: str) -> int:
    result = 0x811C9DC5
    for byte in value.encode("ascii", errors="strict"):
        result ^= byte
        result = (result * 0x01000193) & UINT32_MAX
    return result


def npc_name_hash(value: str) -> int:
    canonical = canonicalize_npc_name(value)
    return fnv1a_32(canonical) if canonical else 0


def _state_components_by_achievement(
    resources: ResourceSet,
) -> dict[int, List[Component]]:
    result: dict[int, List[Component]] = {}
    for component in resources.components:
        if component.component_type <= 2:
            result.setdefault(component.achievement_id, []).append(component)
    for components in result.values():
        components.sort(
            key=lambda component: (
                component.sequence,
                component.component_type,
                component.component_id,
            )
        )
    return result


def _required_counts(resources: ResourceSet) -> dict[int, int]:
    return {
        row.component_id: row.required_count
        for row in resources.component_counts
    }


def _criterion_behavior(component_type: int) -> Optional[int]:
    if component_type == 1:
        return BEHAVIOR_REQUIRED
    if component_type == 2:
        return BEHAVIOR_OPTIONAL
    return None


def _level_evaluation_criterion(
    criterion: LevelCriterion,
) -> EvaluationCriterion:
    return EvaluationCriterion(
        achievement_id=criterion.achievement_id,
        component_type=criterion.component_type,
        component_sequence=criterion.component_sequence,
        component_id=criterion.component_id,
        event_type=EVENT_LEVEL,
        progress_mode=PROGRESS_BOOLEAN,
        behavior=BEHAVIOR_REQUIRED,
        target_id=0,
        target_id2=0,
        target_value=criterion.level,
    )


def _criterion_component_key(
    criterion: EvaluationCriterion,
) -> Tuple[int, int, int]:
    return (
        criterion.achievement_id,
        criterion.component_type,
        criterion.component_id,
    )


def _criterion_policy(
    criterion: EvaluationCriterion,
) -> Tuple[int, int, int, int]:
    return (
        criterion.behavior,
        criterion.required_count,
        criterion.event_type,
        criterion.progress_mode,
    )


def _validated_component_policies(
    criteria: Sequence[EvaluationCriterion],
    item_name_criteria: Sequence[ItemNameCriterion] = (),
) -> dict[Tuple[int, int, int], Tuple[int, int, int, int]]:
    policies: dict[Tuple[int, int, int], Tuple[int, int, int, int]] = {}
    criterion_identities: set[Tuple[int, int, int, int, int, int]] = set()

    for criterion in criteria:
        identity = (
            criterion.achievement_id,
            criterion.component_type,
            criterion.component_id,
            criterion.event_type,
            criterion.target_id,
            criterion.target_id2,
        )
        if identity in criterion_identities:
            raise ResourceError(
                "generated duplicate achievement criterion identity "
                f"{identity}"
            )
        criterion_identities.add(identity)

        component_key = _criterion_component_key(criterion)
        policy = _criterion_policy(criterion)
        existing = policies.setdefault(component_key, policy)
        if existing != policy:
            raise ResourceError(
                "generated criteria for component "
                f"{component_key} have conflicting behavior, required-count, "
                "event, or progress-mode policy"
            )

    for criterion in item_name_criteria:
        component_key = (
            criterion.achievement_id,
            criterion.component_type,
            criterion.component_id,
        )
        policy = (
            criterion.behavior,
            criterion.required_count,
            EVENT_OWN_ITEM,
            PROGRESS_BOOLEAN,
        )
        existing = policies.setdefault(component_key, policy)
        if existing != policy:
            raise ResourceError(
                "generated criteria for component "
                f"{component_key} have conflicting behavior, required-count, "
                "event, or progress-mode policy"
            )

    return policies


def _component_shape_digest(
    components: Sequence[Component],
    component_counts: dict[int, int],
) -> str:
    rows = (
        f"{component.sequence}\t{component.component_type}\t"
        f"{component.component_id}\t{component.name}\t"
        f"{component_counts.get(component.component_id, 1)}"
        for component in sorted(
            components,
            key=lambda row: (
                row.component_type,
                row.sequence,
                row.component_id,
            ),
        )
    )
    return hashlib.sha256("\n".join(rows).encode("utf-8")).hexdigest()


def _reviewed_source_criteria(
    resources: ResourceSet,
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> Tuple[
    Tuple[EvaluationCriterion, ...],
    Tuple[Tuple[int, int, int], ...],
    Tuple[str, ...],
]:
    achievements = {
        row.achievement_id: row for row in resources.achievements
    }
    components = {
        (
            component.achievement_id,
            component.component_type,
            component.component_id,
        ): component
        for rows in components_by_achievement.values()
        for component in rows
    }
    criteria: List[EvaluationCriterion] = []
    rejections: List[str] = []

    def append_targets(
        key: Tuple[int, int, int],
        targets: Sequence[int],
        event_type: int,
        target_value: int,
    ) -> None:
        component = components.get(key)
        if component is None:
            return
        behavior = _criterion_behavior(component.component_type)
        if (
            behavior is None
            or component_counts.get(component.component_id, 1) != 1
            or not targets
        ):
            rejections.append(
                f"reviewed mapping {key} no longer has its audited component policy"
            )
            return
        for target_id in targets:
            if target_id <= 0:
                rejections.append(
                    f"reviewed mapping {key} has invalid target {target_id}"
                )
                continue
            if (
                event_type == EVENT_ACHIEVEMENT_COMPLETE
                and target_id not in achievements
            ):
                rejections.append(
                    f"reviewed dependency {key} targets missing achievement "
                    f"{target_id}"
                )
                continue
            criteria.append(
                EvaluationCriterion(
                    achievement_id=component.achievement_id,
                    component_type=component.component_type,
                    component_sequence=component.sequence,
                    component_id=component.component_id,
                    event_type=event_type,
                    progress_mode=PROGRESS_BOOLEAN,
                    behavior=behavior,
                    target_id=target_id,
                    target_id2=0,
                    target_value=target_value,
                )
            )

    for achievement_id, expected_digest in REVIEWED_RAID_SHAPE_DIGESTS.items():
        state_components = components_by_achievement.get(achievement_id, ())
        if not state_components:
            continue
        actual_digest = _component_shape_digest(
            state_components, component_counts
        )
        if actual_digest != expected_digest:
            rejections.append(
                f"raid definition {achievement_id} shape changed "
                f"({actual_digest}); named-kill mappings omitted"
            )
            continue
        zone_id = REVIEWED_RAID_ZONE_OVERRIDES.get(
            achievement_id, (achievement_id // 100) % 1000
        )
        for component in state_components:
            key = (
                achievement_id,
                component.component_type,
                component.component_id,
            )
            if (
                key in REVIEWED_RAID_SCRIPT_COMPONENTS
                or key in REVIEWED_RAID_UNAVAILABLE_COMPONENTS
            ):
                continue
            behavior = _criterion_behavior(component.component_type)
            npc_names = REVIEWED_NPC_NAME_TARGETS.get(
                key,
                (
                    REVIEWED_NPC_NAME_OVERRIDES.get(
                        key, component.name
                    ),
                ),
            )
            if (
                behavior is None
                or component_counts.get(component.component_id, 1) != 1
                or zone_id <= 0
                or not npc_names
                or any(npc_name_hash(name) == 0 for name in npc_names)
            ):
                rejections.append(
                    f"reviewed raid component {key} no longer maps safely"
                )
                continue
            for npc_name in npc_names:
                criteria.append(
                    EvaluationCriterion(
                        achievement_id=achievement_id,
                        component_type=component.component_type,
                        component_sequence=component.sequence,
                        component_id=component.component_id,
                        event_type=EVENT_NPC_NAME_KILL,
                        progress_mode=PROGRESS_BOOLEAN,
                        behavior=behavior,
                        target_id=npc_name_hash(npc_name),
                        target_id2=zone_id,
                        target_value=0,
                    )
                )

    for achievement_id, expected_digest in REVIEWED_HUNTER_SHAPE_DIGESTS.items():
        state_components = components_by_achievement.get(achievement_id, ())
        if not state_components:
            continue
        actual_digest = _component_shape_digest(
            state_components, component_counts
        )
        if actual_digest != expected_digest:
            rejections.append(
                f"Hunter definition {achievement_id} shape changed "
                f"({actual_digest}); named-kill mappings omitted"
            )
            continue
        for component in state_components:
            key = (
                achievement_id,
                component.component_type,
                component.component_id,
            )
            if key in REVIEWED_HUNTER_UNAVAILABLE_COMPONENTS:
                continue
            zone_id = REVIEWED_HUNTER_COMPONENT_ZONES.get(
                key, REVIEWED_HUNTER_DEFAULT_ZONES.get(achievement_id, 0)
            )
            behavior = _criterion_behavior(component.component_type)
            npc_name = REVIEWED_NPC_NAME_OVERRIDES.get(
                key, component.name
            )
            target_id = npc_name_hash(npc_name)
            if (
                behavior is None
                or component_counts.get(component.component_id, 1) != 1
                or zone_id <= 0
                or target_id == 0
            ):
                rejections.append(
                    f"reviewed Hunter component {key} no longer maps safely"
                )
                continue
            criteria.append(
                EvaluationCriterion(
                    achievement_id=achievement_id,
                    component_type=component.component_type,
                    component_sequence=component.sequence,
                    component_id=component.component_id,
                    event_type=EVENT_NPC_NAME_KILL,
                    progress_mode=PROGRESS_BOOLEAN,
                    behavior=behavior,
                    target_id=target_id,
                    target_id2=zone_id,
                    target_value=0,
                )
            )

    for key, item_ids in REVIEWED_OWN_ITEM_TARGETS.items():
        append_targets(key, item_ids, EVENT_OWN_ITEM, 1)
    for key, task_ids in REVIEWED_TASK_TARGETS.items():
        append_targets(key, task_ids, EVENT_TASK_COMPLETE, 0)
    for key, dependency_ids in REVIEWED_DEPENDENCY_TARGETS.items():
        append_targets(
            key, dependency_ids, EVENT_ACHIEVEMENT_COMPLETE, 0
        )

    source_keys = tuple(
        sorted({_criterion_component_key(criterion) for criterion in criteria})
    )
    return tuple(criteria), source_keys, tuple(sorted(set(rejections)))


def _reviewed_coverage(
    resources: ResourceSet,
    source_keys: Sequence[Tuple[int, int, int]],
) -> Tuple[
    Tuple[Tuple[int, int, int], ...],
    Tuple[Tuple[int, int, int], ...],
    Tuple[Tuple[int, int, int], ...],
    Tuple[Tuple[int, int, int], ...],
]:
    available_keys = {
        (
            component.achievement_id,
            component.component_type,
            component.component_id,
        )
        for component in resources.components
        if component.component_type <= 2
    }
    source = set(source_keys)
    script = set(REVIEWED_SCRIPT_COMPONENTS) & available_keys
    presentation = set(REVIEWED_PRESENTATION_COMPONENTS) & available_keys
    unavailable = (
        set(REVIEWED_PARTIAL_UNAVAILABLE_COMPONENTS)
        | set(REVIEWED_RAID_UNAVAILABLE_COMPONENTS)
        | set(REVIEWED_HUNTER_UNAVAILABLE_COMPONENTS)
        | {
            key
            for key in available_keys
            if key[0] in REVIEWED_UNAVAILABLE_ACHIEVEMENT_IDS
        }
    ) & available_keys
    unavailable -= source | script | presentation

    classified = (source, script, presentation, unavailable)
    for index, left in enumerate(classified):
        for right in classified[index + 1 :]:
            overlap = left & right
            if overlap:
                raise ResourceError(
                    "reviewed achievement coverage classifications overlap: "
                    + ", ".join(
                        f"{achievement_id}/{component_type}/{component_id}"
                        for achievement_id, component_type, component_id
                        in sorted(overlap)
                    )
                )

    return tuple(tuple(sorted(values)) for values in classified)


def _reviewed_script_criteria(
    components_by_achievement: Mapping[int, Sequence[Component]],
    component_counts: Mapping[int, int],
    component_keys: Sequence[Tuple[int, int, int]],
) -> Tuple[EvaluationCriterion, ...]:
    components = {
        (
            component.achievement_id,
            component.component_type,
            component.component_id,
        ): component
        for rows in components_by_achievement.values()
        for component in rows
    }
    criteria: List[EvaluationCriterion] = []
    for key in sorted(set(component_keys)):
        component = components.get(key)
        behavior = (
            _criterion_behavior(component.component_type)
            if component is not None
            else None
        )
        if component is None or behavior is None:
            raise ResourceError(
                "reviewed quest-manual component no longer has a valid "
                f"state-bearing policy: {key}"
            )

        criteria.append(
            EvaluationCriterion(
                achievement_id=component.achievement_id,
                component_type=component.component_type,
                component_sequence=component.sequence,
                component_id=component.component_id,
                event_type=EVENT_MANUAL,
                progress_mode=PROGRESS_SET,
                behavior=behavior,
                target_id=0,
                target_id2=0,
                target_value=0,
                required_count=component_counts.get(component.component_id, 1),
            )
        )
    return tuple(criteria)


def _without_reviewed_item_name_criteria(
    criteria: Sequence[ItemNameCriterion],
) -> Tuple[ItemNameCriterion, ...]:
    reviewed_keys = set(REVIEWED_OWN_ITEM_TARGETS) | set(
        REVIEWED_SCRIPT_COMPONENTS
    )
    return tuple(
        criterion
        for criterion in criteria
        if (
            criterion.achievement_id,
            criterion.component_type,
            criterion.component_id,
        )
        not in reviewed_keys
    )


def _general_category_achievement_ids(
    resources: ResourceSet,
    category_name: str,
) -> frozenset[int]:
    general_roots = [
        row.category_id
        for row in resources.categories
        if row.parent_id == 0 and row.name.casefold() == "general"
    ]
    if len(general_roots) != 1:
        return frozenset()

    general_category_ids = _category_descendants(
        resources, {general_roots[0]}
    )
    matching_categories = [
        row.category_id
        for row in resources.categories
        if row.category_id in general_category_ids
        and row.name.casefold() == category_name.casefold()
    ]
    if not matching_categories:
        return frozenset()
    if len(matching_categories) != 1:
        raise ResourceError(
            f"expected at most one 'General / {category_name}' achievement "
            f"category, found {len(matching_categories)}"
        )

    category_ids = _category_descendants(
        resources, {matching_categories[0]}
    )
    achievement_ids = {
        row.achievement_id for row in resources.achievements
    }
    return frozenset(
        association.achievement_id
        for association in resources.category_associations
        if association.category_id in category_ids
        and association.achievement_id in achievement_ids
    )


def _progression_level_milestones(
    resources: ResourceSet,
    max_level: int,
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> Tuple[Tuple[int, ...], Tuple[LevelCriterion, ...], int, int]:
    achievement_ids = _general_category_achievement_ids(
        resources, "Progression"
    )
    achievements = {
        row.achievement_id: row for row in resources.achievements
    }
    presentation_components: dict[int, List[Component]] = {}
    for component in resources.components:
        if component.component_type == 3:
            presentation_components.setdefault(
                component.achievement_id, []
            ).append(component)

    selected: List[int] = []
    criteria: List[LevelCriterion] = []
    candidates = 0
    rejected = 0
    for achievement_id in sorted(achievement_ids):
        achievement = achievements[achievement_id]
        name_match = re.fullmatch(
            r"Reach Level ([1-9][0-9]*)", achievement.name
        )
        if name_match is None:
            continue
        level = int(name_match.group(1), 10)
        if level > max_level:
            continue
        candidates += 1

        state_components = components_by_achievement.get(
            achievement_id, ()
        )
        gates = presentation_components.get(achievement_id, ())
        valid = (
            achievement.description == f"Reach Level {level}"
            and len(state_components) == 1
            and state_components[0].component_type == 1
            and state_components[0].sequence == 0
            and state_components[0].component_id == achievement_id
            and state_components[0].name == f"Reach level {level}"
            and component_counts.get(
                state_components[0].component_id, 1
            )
            == 1
            and len(gates) == 1
            and gates[0].sequence == 0
            and gates[0].component_id == 10014
            and gates[0].name == "On a Level Locked Server"
        )
        if not valid:
            rejected += 1
            continue

        component = state_components[0]
        selected.append(achievement_id)
        criteria.append(
            LevelCriterion(
                achievement_id=achievement_id,
                component_type=component.component_type,
                component_sequence=component.sequence,
                component_id=component.component_id,
                level=level,
            )
        )

    return tuple(selected), tuple(criteria), candidates, rejected


def _skill_cap_milestones(
    resources: ResourceSet,
    max_level: int,
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> Tuple[
    Tuple[int, ...],
    Tuple[EvaluationCriterion, ...],
    int,
    int,
]:
    achievement_ids = _general_category_achievement_ids(
        resources, "Skills"
    )
    achievements = {
        row.achievement_id: row for row in resources.achievements
    }
    presentation_components: dict[int, List[Component]] = {}
    for component in resources.components:
        if component.component_type == 3:
            presentation_components.setdefault(
                component.achievement_id, []
            ).append(component)

    selected: List[int] = []
    criteria: List[EvaluationCriterion] = []
    candidates = 0
    rejected = 0
    for achievement_id in sorted(achievement_ids):
        achievement = achievements[achievement_id]
        name_match = re.fullmatch(
            r".+, Level ([1-9][0-9]*)", achievement.name
        )
        if name_match is None:
            continue
        milestone_level = int(name_match.group(1), 10)
        if milestone_level > max_level:
            continue
        candidates += 1

        gates = presentation_components.get(achievement_id, ())
        class_match = (
            re.fullmatch(
                r"Meet or Exceed the Class Requirement \(([^)]+)\)",
                gates[0].name,
            )
            if len(gates) == 1
            else None
        )
        class_id = (
            CLASS_IDS.get(class_match.group(1))
            if class_match is not None
            else None
        )
        description_match = re.fullmatch(
            r"This achievement is completed by reaching the maximum skill "
            r"in all (?:casting|combat|instrument|specialized|utility|weapon) "
            r"skills at level ([1-9][0-9]*)\.",
            achievement.description,
        )
        state_components = components_by_achievement.get(
            achievement_id, ()
        )
        definition_criteria: List[EvaluationCriterion] = []
        seen_skills: set[int] = set()
        valid = (
            class_id is not None
            and description_match is not None
            and int(description_match.group(1), 10) == milestone_level
            and bool(state_components)
            and gates[0].sequence == 0
            and gates[0].component_id == 1302 + class_id
        )
        for component in state_components:
            behavior = _criterion_behavior(component.component_type)
            component_match = re.fullmatch(
                r"Reach the maximum skill in (.+) at level "
                r"([1-9][0-9]*)\.",
                component.name,
            )
            skill_name = (
                component_match.group(1)
                if component_match is not None
                else ""
            )
            component_level = (
                int(component_match.group(2), 10)
                if component_match is not None
                else 0
            )
            skill_id = SKILL_CAP_SKILL_IDS.get(skill_name)
            if (
                behavior is None
                or skill_id is None
                or component_level != milestone_level
                or skill_id in seen_skills
                or component_counts.get(component.component_id, 1) != 1
            ):
                valid = False
                break
            seen_skills.add(skill_id)
            definition_criteria.append(
                EvaluationCriterion(
                    achievement_id=achievement_id,
                    component_type=component.component_type,
                    component_sequence=component.sequence,
                    component_id=component.component_id,
                    event_type=EVENT_SKILL_CAP,
                    progress_mode=PROGRESS_BOOLEAN,
                    behavior=behavior,
                    target_id=skill_id,
                    target_id2=class_id,
                    target_value=milestone_level,
                )
            )

        if not valid:
            rejected += 1
            continue
        selected.append(achievement_id)
        criteria.extend(definition_criteria)

    return tuple(selected), tuple(criteria), candidates, rejected


def _traveler_criteria(
    resources: ResourceSet,
    selected_ids: frozenset[int],
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> Tuple[
    Tuple[EvaluationCriterion, ...],
    dict[str, List[Tuple[int, Tuple[int, ...]]]],
    int,
    int,
]:
    achievements = {
        row.achievement_id: row
        for row in resources.achievements
        if row.achievement_id in selected_ids
    }
    criteria: List[EvaluationCriterion] = []
    locations: dict[str, List[Tuple[int, Tuple[int, ...]]]] = {}
    candidates = 0
    rejected = 0

    for achievement_id in sorted(achievements):
        achievement = achievements[achievement_id]
        name_match = re.fullmatch(r"(.+) Traveler", achievement.name)
        if name_match is None:
            continue
        candidates += 1
        location = name_match.group(1)
        components = components_by_achievement.get(achievement_id, [])
        base_zone_id = (achievement_id // 100) % 1000
        zone_ids: Tuple[int, ...] = ()

        if (
            achievement_id == 930400
            and achievement.name
            == "Muramite Proving Grounds Trials Traveler"
        ):
            valid = (
                achievement_id % 100 == 0
                and base_zone_id == 304
                and len(components) == 6
                and [row.sequence for row in components]
                == list(range(1, 7))
                and all(row.component_type == 1 for row in components)
                and all(
                    re.fullmatch(r"Visit .+", row.name) is not None
                    for row in components
                )
                and all(
                    component_counts.get(row.component_id, 1) == 1
                    for row in components
                )
            )
            if valid:
                zone_ids = tuple(
                    base_zone_id + row.sequence - 1
                    for row in components
                )
        else:
            valid = (
                achievement_id % 100 == 0
                and base_zone_id != 0
                and len(components) == 1
                and components[0].component_type == 1
                and components[0].sequence == 1
                and re.fullmatch(
                    r"Visit .+", components[0].name
                )
                is not None
                and component_counts.get(components[0].component_id, 1) == 1
            )
            if valid:
                zone_ids = (base_zone_id,)

        if not zone_ids:
            rejected += 1
            continue

        locations.setdefault(location, []).append(
            (achievement_id, zone_ids)
        )
        for component, zone_id in zip(components, zone_ids):
            criteria.append(
                EvaluationCriterion(
                    achievement_id=achievement_id,
                    component_type=component.component_type,
                    component_sequence=component.sequence,
                    component_id=component.component_id,
                    event_type=EVENT_ZONE_ENTER,
                    progress_mode=PROGRESS_BOOLEAN,
                    behavior=BEHAVIOR_REQUIRED,
                    target_id=zone_id,
                    target_id2=0,
                    target_value=0,
                )
            )

    return tuple(criteria), locations, candidates, rejected


def _reject_dependency_cycles(
    criteria: Sequence[EvaluationCriterion],
) -> None:
    adjacency: dict[int, set[int]] = {}
    for criterion in criteria:
        adjacency.setdefault(criterion.achievement_id, set()).add(
            criterion.target_id
        )

    state: dict[int, int] = {}
    stack: List[int] = []
    stack_positions: dict[int, int] = {}

    def visit(achievement_id: int) -> None:
        state[achievement_id] = 1
        stack_positions[achievement_id] = len(stack)
        stack.append(achievement_id)
        for dependency_id in sorted(adjacency.get(achievement_id, ())):
            dependency_state = state.get(dependency_id, 0)
            if dependency_state == 0:
                visit(dependency_id)
            elif dependency_state == 1:
                cycle_start = stack_positions[dependency_id]
                cycle = stack[cycle_start:] + [dependency_id]
                raise ResourceError(
                    "generated AchievementComplete criteria contain a "
                    "dependency cycle: "
                    + " -> ".join(str(value) for value in cycle)
                )
        stack.pop()
        stack_positions.pop(achievement_id)
        state[achievement_id] = 2

    for achievement_id in sorted(adjacency):
        if state.get(achievement_id, 0) == 0:
            visit(achievement_id)


def _achievement_dependency_criteria(
    resources: ResourceSet,
    selected_ids: frozenset[int],
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
    claimed_component_policies: dict[
        Tuple[int, int, int], Tuple[int, int, int, int]
    ],
) -> Tuple[
    Tuple[EvaluationCriterion, ...],
    Tuple[EvaluationCriterion, ...],
    int,
    int,
    int,
]:
    names: dict[str, List[int]] = {}
    for achievement in resources.achievements:
        if achievement.achievement_id in selected_ids:
            names.setdefault(achievement.name, []).append(
                achievement.achievement_id
            )
    for achievement_ids in names.values():
        achievement_ids.sort()

    criteria: List[EvaluationCriterion] = []
    superseded: List[EvaluationCriterion] = []
    ambiguous = 0
    self_references = 0
    rejected = 0
    for achievement_id in sorted(selected_ids):
        for component in components_by_achievement.get(achievement_id, ()):
            matches = names.get(component.name, ())
            if not matches:
                continue
            if len(matches) != 1:
                ambiguous += 1
                continue
            dependency_id = matches[0]
            if dependency_id == achievement_id:
                self_references += 1
                continue
            behavior = _criterion_behavior(component.component_type)
            if (
                behavior is None
                or component_counts.get(component.component_id, 1) != 1
            ):
                rejected += 1
                continue
            criterion = EvaluationCriterion(
                achievement_id=achievement_id,
                component_type=component.component_type,
                component_sequence=component.sequence,
                component_id=component.component_id,
                event_type=EVENT_ACHIEVEMENT_COMPLETE,
                progress_mode=PROGRESS_BOOLEAN,
                behavior=behavior,
                target_id=dependency_id,
                target_id2=0,
                target_value=0,
            )
            claimed_policy = claimed_component_policies.get(
                _criterion_component_key(criterion)
            )
            if (
                claimed_policy is not None
                and claimed_policy != _criterion_policy(criterion)
            ):
                # Structurally validated direct event criteria take precedence
                # over this name-only dependency inference. Retain the exact
                # suppressed identity so a rerun can disable rows emitted by an
                # older importer.
                superseded.append(criterion)
                continue
            criteria.append(criterion)

    _reject_dependency_cycles(criteria)
    return (
        tuple(criteria),
        tuple(superseded),
        ambiguous,
        self_references,
        rejected,
    )


def _slayer_race_ids(
    description: str,
) -> Tuple[Tuple[int, ...], Tuple[str, ...]]:
    override = SLAYER_RACE_DESCRIPTION_OVERRIDES.get(description)
    if override is not None:
        return tuple(sorted(set(override))), ()

    value = description.rstrip(".")
    terms = tuple(
        term.strip()
        for term in re.split(r",\s*(?:and\s+)?|\s+and\s+", value)
        if term.strip()
    )
    if not terms:
        return (), (description,)

    unresolved = tuple(
        term
        for term in terms
        if not SLAYER_RACE_TERM_IDS.get(term)
    )
    if unresolved:
        return (), unresolved

    race_ids = tuple(
        sorted(
            {
                race_id
                for term in terms
                for race_id in SLAYER_RACE_TERM_IDS[term]
            }
        )
    )
    return race_ids, ()


def _slayer_dependency_name(
    component: Component,
) -> Optional[str]:
    if component.component_type == 1:
        pattern = r'Complete the achievement "([^"]+)"'
    elif component.component_type == 2:
        pattern = r'\(Optional\) Complete the achievement "([^"]+)"'
    else:
        return None
    match = re.fullmatch(pattern, component.name)
    return match.group(1) if match is not None else None


def _slayer_name_key(value: str) -> str:
    # The ToB resources vary only terminal display punctuation and case for
    # otherwise-identical names.
    return value.casefold().rstrip(".!?")


def _slayer_dependency_key(value: str) -> str:
    # Resolve only the audited stale dependency labels. Canonical definition
    # names are indexed without aliases so an unexpected duplicate or renamed
    # definition cannot silently become a match.
    key = _slayer_name_key(value)
    canonical_name = SLAYER_DEPENDENCY_NAME_ALIASES.get(key)
    return (
        _slayer_name_key(canonical_name)
        if canonical_name is not None
        else key
    )


def _slayer_criteria(
    resources: ResourceSet,
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> Tuple[
    Tuple[int, ...],
    Tuple[EvaluationCriterion, ...],
    Tuple[str, ...],
    int,
    int,
    int,
]:
    roots = [
        category.category_id
        for category in resources.categories
        if category.parent_id == 0
        and category.name.casefold() == "slayer"
    ]
    if len(roots) != 1:
        raise ResourceError(
            "expected exactly one top-level 'Slayer' achievement category, "
            f"found {len(roots)}"
        )

    category_ids = _category_descendants(resources, {roots[0]})
    achievement_ids = {row.achievement_id for row in resources.achievements}
    candidate_ids = {
        association.achievement_id
        for association in resources.category_associations
        if association.category_id in category_ids
        and association.achievement_id in achievement_ids
    }
    achievements = {
        achievement.achievement_id: achievement
        for achievement in resources.achievements
        if achievement.achievement_id in candidate_ids
    }
    names: dict[str, List[int]] = {}
    for achievement_id in sorted(candidate_ids):
        names.setdefault(
            _slayer_name_key(
                achievements[achievement_id].name
            ),
            [],
        ).append(achievement_id)

    selected_ids: set[int] = set()
    criteria: List[EvaluationCriterion] = []
    rejections: dict[int, str] = {}
    meta_components: dict[int, Tuple[Tuple[Component, str], ...]] = {}
    direct_selected = 0

    for achievement_id in sorted(candidate_ids):
        achievement = achievements[achievement_id]
        components = tuple(
            components_by_achievement.get(achievement_id, ())
        )
        if not components:
            rejections[achievement_id] = (
                f"Slayer achievement {achievement_id} "
                f"({achievement.name!r}) has no state-bearing components"
            )
            continue

        dependencies = tuple(
            (component, _slayer_dependency_name(component))
            for component in components
        )
        dependency_count = sum(
            dependency_name is not None
            for _component, dependency_name in dependencies
        )
        if dependency_count:
            if dependency_count != len(components):
                rejections[achievement_id] = (
                    f"Slayer achievement {achievement_id} "
                    f"({achievement.name!r}) mixes quoted achievement "
                    "dependencies with direct race components"
                )
                continue
            meta_components[achievement_id] = tuple(
                (component, dependency_name)
                for component, dependency_name in dependencies
                if dependency_name is not None
            )
            continue

        definition_criteria: List[EvaluationCriterion] = []
        failure: Optional[str] = None
        for component in components:
            behavior = _criterion_behavior(component.component_type)
            required_count = component_counts.get(
                component.component_id, 1
            )
            race_ids, unresolved = _slayer_race_ids(
                component.name
            )
            if behavior is None:
                failure = (
                    f"component {component.component_id} has unsupported "
                    f"type {component.component_type}"
                )
                break
            if required_count < 1:
                failure = (
                    f"component {component.component_id} has invalid "
                    f"required count {required_count}"
                )
                break
            if unresolved:
                failure = (
                    f"component {component.component_id} has unmapped race "
                    f"term(s) {', '.join(repr(value) for value in unresolved)} "
                    f"in {component.name!r}"
                )
                break
            if not race_ids:
                failure = (
                    f"component {component.component_id} resolved to no "
                    f"RoF2 race IDs from {component.name!r}"
                )
                break
            for race_id in race_ids:
                definition_criteria.append(
                    EvaluationCriterion(
                        achievement_id=achievement_id,
                        component_type=component.component_type,
                        component_sequence=component.sequence,
                        component_id=component.component_id,
                        event_type=EVENT_NPC_RACE_KILL,
                        progress_mode=PROGRESS_INCREMENT,
                        behavior=behavior,
                        target_id=race_id,
                        target_id2=0,
                        target_value=0,
                        required_count=required_count,
                    )
                )

        if failure is not None:
            rejections[achievement_id] = (
                f"Slayer achievement {achievement_id} "
                f"({achievement.name!r}) rejected: {failure}"
            )
            continue

        selected_ids.add(achievement_id)
        direct_selected += 1
        criteria.extend(definition_criteria)

    pending = set(meta_components)
    meta_selected = 0
    while pending:
        progressed = False
        for achievement_id in sorted(tuple(pending)):
            achievement = achievements[achievement_id]
            resolved: List[Tuple[Component, int]] = []
            unresolved_required: List[str] = []
            permanently_invalid: List[str] = []
            for component, dependency_name in meta_components[
                achievement_id
            ]:
                matches = names.get(
                    _slayer_dependency_key(dependency_name), ()
                )
                behavior = _criterion_behavior(component.component_type)
                required_count = component_counts.get(
                    component.component_id, 1
                )
                if behavior is None or required_count != 1:
                    permanently_invalid.append(
                        f"component {component.component_id} has invalid "
                        "dependency type or required count"
                    )
                    continue
                if len(matches) != 1:
                    if component.component_type == 1:
                        permanently_invalid.append(
                            f"required dependency {dependency_name!r} "
                            f"matched {len(matches)} Slayer definitions"
                        )
                    continue
                dependency_id = matches[0]
                if dependency_id == achievement_id:
                    permanently_invalid.append(
                        f"component {component.component_id} is self-referential"
                    )
                    continue
                if dependency_id not in selected_ids:
                    if component.component_type == 1:
                        unresolved_required.append(dependency_name)
                    continue
                resolved.append((component, dependency_id))

            if permanently_invalid:
                rejections[achievement_id] = (
                    f"Slayer achievement {achievement_id} "
                    f"({achievement.name!r}) rejected: "
                    + "; ".join(permanently_invalid)
                )
                pending.remove(achievement_id)
                progressed = True
                continue
            if unresolved_required:
                continue

            selected_ids.add(achievement_id)
            pending.remove(achievement_id)
            progressed = True
            meta_selected += 1
            for component, dependency_id in resolved:
                criteria.append(
                    EvaluationCriterion(
                        achievement_id=achievement_id,
                        component_type=component.component_type,
                        component_sequence=component.sequence,
                        component_id=component.component_id,
                        event_type=EVENT_ACHIEVEMENT_COMPLETE,
                        progress_mode=PROGRESS_BOOLEAN,
                        behavior=_criterion_behavior(
                            component.component_type
                        ),
                        target_id=dependency_id,
                        target_id2=0,
                        target_value=0,
                    )
                )

        if not progressed:
            for achievement_id in sorted(pending):
                achievement = achievements[achievement_id]
                unavailable = []
                for component, dependency_name in meta_components[
                    achievement_id
                ]:
                    if component.component_type != 1:
                        continue
                    matches = names.get(
                        _slayer_dependency_key(dependency_name), ()
                    )
                    if len(matches) != 1:
                        unavailable.append(
                            f"{dependency_name!r} "
                            f"({len(matches)} name matches)"
                        )
                    elif matches[0] not in selected_ids:
                        unavailable.append(
                            f"{dependency_name!r} "
                            f"(definition {matches[0]} not safely selected)"
                        )
                rejections[achievement_id] = (
                    f"Slayer achievement {achievement_id} "
                    f"({achievement.name!r}) rejected: required "
                    "dependencies are unavailable: "
                    + ", ".join(unavailable)
                )
            pending.clear()

    dependency_criteria = tuple(
        criterion
        for criterion in criteria
        if criterion.event_type == EVENT_ACHIEVEMENT_COMPLETE
    )
    _reject_dependency_cycles(dependency_criteria)
    return (
        tuple(sorted(selected_ids)),
        tuple(criteria),
        tuple(rejections[achievement_id] for achievement_id in sorted(rejections)),
        len(candidate_ids),
        direct_selected,
        meta_selected,
    )


def _filter_don_historical_selection(
    resources: ResourceSet,
    selected_ids: set[int],
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> set[int]:
    """Fail closed on content that a current client files under old roots."""
    if not selected_ids:
        return set()

    (
        traveler_criteria,
        traveler_locations,
        _traveler_candidates,
        _traveler_rejected,
    ) = _traveler_criteria(
        resources,
        frozenset(selected_ids),
        components_by_achievement,
        component_counts,
    )
    traveler_zones: dict[int, set[int]] = {}
    for criterion in traveler_criteria:
        traveler_zones.setdefault(criterion.achievement_id, set()).add(
            criterion.target_id
        )

    blocked_travelers = {
        achievement_id
        for achievement_id, zone_ids in traveler_zones.items()
        if any(
            zone_id > DON_MAX_HISTORICAL_ZONE_ID
            or zone_id in DON_EXCLUDED_ZONE_IDS
            for zone_id in zone_ids
        )
    }
    blocked_families = {
        achievement_id // 100 for achievement_id in blocked_travelers
    }
    blocked = {
        achievement_id
        for achievement_id in selected_ids
        if achievement_id // 100 in blocked_families
    }

    # The current snapshot files enhanced revamps under their original
    # expansion's Hunter category. Their availability components are the one
    # explicit client-side version discriminator.
    blocked.update(
        component.achievement_id
        for component in resources.components
        if component.achievement_id in selected_ids
        and component.component_type == 3
        and ENHANCED_AVAILABILITY_MARKER in component.name
    )

    allowed_locations: set[str] = set()
    blocked_locations: set[str] = set()
    for location, matches in traveler_locations.items():
        if any(
            achievement_id not in blocked_travelers
            for achievement_id, _zone_ids in matches
        ):
            allowed_locations.add(location)
        if any(
            achievement_id in blocked_travelers
            for achievement_id, _zone_ids in matches
        ):
            blocked_locations.add(location)

    achievements_by_id = {
        achievement.achievement_id: achievement
        for achievement in resources.achievements
    }
    for achievement_id in selected_ids:
        achievement = achievements_by_id.get(achievement_id)
        if achievement is None:
            continue
        match = re.fullmatch(
            r"(?:Hunter|Conqueror) of (.+)",
            achievement.name,
        )
        if (
            match is not None
            and match.group(1) in blocked_locations
            and match.group(1) not in allowed_locations
        ):
            blocked.add(achievement_id)

    # Exact-name components are the client resource's only structural parent
    # relation. Removing a required child must recursively remove every parent
    # that would otherwise be permanently impossible. Optional children do
    # not prevent the parent from remaining available.
    achievement_ids_by_name: dict[str, set[int]] = {}
    for achievement in resources.achievements:
        achievement_ids_by_name.setdefault(achievement.name, set()).add(
            achievement.achievement_id
        )

    changed = True
    while changed:
        changed = False
        remaining = selected_ids - blocked
        for achievement_id in sorted(remaining):
            for component in components_by_achievement.get(
                achievement_id, ()
            ):
                if component.component_type != 1:
                    continue
                dependencies = (
                    achievement_ids_by_name.get(
                        component.name, set()
                    )
                    - {achievement_id}
                )
                if dependencies and not dependencies.intersection(remaining):
                    blocked.add(achievement_id)
                    changed = True
                    break

    return selected_ids - blocked


def _category_has_named_ancestor(
    category_id: int,
    categories_by_id: dict[int, Category],
    names: frozenset[str],
) -> bool:
    visited: set[int] = set()
    while category_id and category_id not in visited:
        visited.add(category_id)
        category = categories_by_id.get(category_id)
        if category is None:
            return False
        if category.name.casefold() in names:
            return True
        category_id = category.parent_id
    return False


def _npc_name_criteria(
    resources: ResourceSet,
    selected_ids: frozenset[int],
    traveler_locations: dict[str, List[Tuple[int, Tuple[int, ...]]]],
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> Tuple[Tuple[EvaluationCriterion, ...], int, int, int, int]:
    categories_by_id = {
        row.category_id: row for row in resources.categories
    }
    hunter_definition_ids = {
        association.achievement_id
        for association in resources.category_associations
        if association.achievement_id in selected_ids
        and _category_has_named_ancestor(
            association.category_id,
            categories_by_id,
            HUNTER_CATEGORY_NAMES,
        )
    }
    selected_names = {
        row.name
        for row in resources.achievements
        if row.achievement_id in selected_ids
    }
    achievements = {
        row.achievement_id: row for row in resources.achievements
    }
    definitions_by_name: dict[str, List[int]] = {}
    for achievement_id in sorted(hunter_definition_ids):
        achievement = achievements.get(achievement_id)
        if (
            achievement is not None
            and re.fullmatch(r"Hunter of .+", achievement.name)
            is not None
        ):
            definitions_by_name.setdefault(
                achievement.name, []
            ).append(achievement_id)

    definitions = 0
    unresolved_definitions = 0
    rejected_components = 0
    candidates: List[Tuple[EvaluationCriterion, str]] = []
    for achievement_id in sorted(hunter_definition_ids):
        achievement = achievements.get(achievement_id)
        if achievement is None:
            continue
        name_match = re.fullmatch(
            r"Hunter of (.+)",
            achievement.name,
        )
        if name_match is None:
            continue
        definitions += 1
        location_matches = traveler_locations.get(
            name_match.group(1), ()
        )
        matching_definitions = definitions_by_name.get(
            achievement.name, ()
        )
        if (
            len(matching_definitions) == 1
            and len(location_matches) == 1
        ):
            resolved_locations = tuple(location_matches)
        else:
            family_definitions = tuple(
                definition_id
                for definition_id in matching_definitions
                if definition_id // 100 == achievement_id // 100
            )
            resolved_locations = (
                tuple(
                    match
                    for match in location_matches
                    if match[0] // 100 == achievement_id // 100
                )
                if len(family_definitions) == 1
                else ()
            )
        if len(resolved_locations) != 1 or len(
            resolved_locations[0][1]
        ) != 1:
            unresolved_definitions += 1
            continue
        zone_id = resolved_locations[0][1][0]

        for component in components_by_achievement.get(
            achievement_id, ()
        ):
            # Meta Hunter rows are handled by AchievementComplete.
            if component.name in selected_names:
                continue
            behavior = _criterion_behavior(component.component_type)
            canonical_name = canonicalize_npc_name(
                component.name
            )
            hashed_name = (
                fnv1a_32(canonical_name) if canonical_name else 0
            )
            if (
                behavior is None
                or component_counts.get(component.component_id, 1) != 1
                or hashed_name == 0
            ):
                rejected_components += 1
                continue
            candidates.append(
                (
                    EvaluationCriterion(
                        achievement_id=achievement_id,
                        component_type=component.component_type,
                        component_sequence=component.sequence,
                        component_id=component.component_id,
                        event_type=EVENT_NPC_NAME_KILL,
                        progress_mode=PROGRESS_BOOLEAN,
                        behavior=behavior,
                        target_id=hashed_name,
                        target_id2=zone_id,
                        target_value=0,
                    ),
                    canonical_name,
                )
            )

    duplicate_definition_keys: set[Tuple[int, int, str]] = set()
    definition_canonical_counts: dict[
        Tuple[int, str], int
    ] = {}
    definition_hash_canonicals: dict[
        Tuple[int, int], set[str]
    ] = {}
    for criterion, canonical_name in candidates:
        canonical_key = (criterion.achievement_id, canonical_name)
        definition_canonical_counts[canonical_key] = (
            definition_canonical_counts.get(canonical_key, 0) + 1
        )
        definition_hash_canonicals.setdefault(
            (criterion.achievement_id, criterion.target_id), set()
        ).add(canonical_name)
    for criterion, canonical_name in candidates:
        if (
            definition_canonical_counts[
                (criterion.achievement_id, canonical_name)
            ]
            > 1
            or len(
                definition_hash_canonicals[
                    (criterion.achievement_id, criterion.target_id)
                ]
            )
            > 1
        ):
            duplicate_definition_keys.add(
                (
                    criterion.achievement_id,
                    criterion.component_id,
                    canonical_name,
                )
            )

    filtered_candidates = tuple(
        (criterion, canonical_name)
        for criterion, canonical_name in candidates
        if (
            criterion.achievement_id,
            criterion.component_id,
            canonical_name,
        )
        not in duplicate_definition_keys
    )

    collision_keys: set[Tuple[int, int]] = set()
    canonicals_by_key: dict[Tuple[int, int], set[str]] = {}
    for criterion, canonical_name in filtered_candidates:
        key = (criterion.target_id2, criterion.target_id)
        canonicals_by_key.setdefault(key, set()).add(canonical_name)
    for key, canonicals in canonicals_by_key.items():
        if len(canonicals) > 1:
            collision_keys.add(key)

    criteria = tuple(
        criterion
        for criterion, _canonical_name in filtered_candidates
        if (criterion.target_id2, criterion.target_id)
        not in collision_keys
    )
    collision_components = len(candidates) - len(criteria)
    return (
        criteria,
        definitions,
        unresolved_definitions,
        rejected_components,
        collision_components,
    )


def _tradeskill_milestones(
    resources: ResourceSet,
    max_skill: int,
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> Tuple[
    Tuple[int, ...],
    Tuple[EvaluationCriterion, ...],
    int,
    int,
]:
    tradeskill_roots = [
        row.category_id
        for row in resources.categories
        if row.parent_id == 0 and row.name.casefold() == "tradeskill"
    ]
    if len(tradeskill_roots) != 1:
        raise ResourceError(
            "expected exactly one top-level 'Tradeskill' achievement "
            f"category, found {len(tradeskill_roots)}"
        )
    category_ids = _category_descendants(
        resources, {tradeskill_roots[0]}
    )
    achievement_ids = {
        association.achievement_id
        for association in resources.category_associations
        if association.category_id in category_ids
    }
    achievements = {
        row.achievement_id: row for row in resources.achievements
    }

    selected: List[int] = []
    criteria: List[EvaluationCriterion] = []
    candidates = 0
    rejected = 0
    for achievement_id in sorted(achievement_ids):
        achievement = achievements.get(achievement_id)
        if achievement is None:
            continue
        name_match = re.fullmatch(
            r"(.+) \(([1-9][0-9]*)\)",
            achievement.name,
        )
        if name_match is None:
            continue
        skill_name = name_match.group(1)
        skill_id = TRADESKILL_SKILL_IDS.get(skill_name)
        if skill_id is None:
            continue
        skill_value = int(name_match.group(2), 10)
        if skill_value > max_skill:
            continue
        candidates += 1

        components = components_by_achievement.get(achievement_id, [])
        expected_component_description = (
            f"Reach {skill_value} skill in {skill_name}"
        )
        expected_achievement_description = (
            "This achievement is completed by reaching "
            f"{skill_value} skill in {skill_name}."
        )
        valid = (
            len(components) == 1
            and components[0].component_type == 1
            and components[0].sequence == 1
            and components[0].component_id == achievement_id
            and components[0].name
            == expected_component_description
            and achievement.description
            == expected_achievement_description
            and component_counts.get(components[0].component_id, 1) == 1
        )
        if not valid:
            rejected += 1
            continue

        component = components[0]
        selected.append(achievement_id)
        criteria.append(
            EvaluationCriterion(
                achievement_id=achievement_id,
                component_type=component.component_type,
                component_sequence=component.sequence,
                component_id=component.component_id,
                event_type=EVENT_SKILL_VALUE,
                progress_mode=PROGRESS_BOOLEAN,
                behavior=BEHAVIOR_REQUIRED,
                target_id=skill_id,
                target_id2=0,
                target_value=skill_value,
            )
        )

    return tuple(selected), tuple(criteria), candidates, rejected


def _aa_spent_milestones(
    resources: ResourceSet,
    max_spent: int,
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> Tuple[
    Tuple[int, ...],
    Tuple[EvaluationCriterion, ...],
    int,
    int,
]:
    general_roots = [
        row.category_id
        for row in resources.categories
        if row.parent_id == 0 and row.name.casefold() == "general"
    ]
    if len(general_roots) != 1:
        raise ResourceError(
            "expected exactly one top-level 'General' achievement category, "
            f"found {len(general_roots)}"
        )
    general_category_ids = _category_descendants(
        resources, {general_roots[0]}
    )
    advancement_categories = [
        row.category_id
        for row in resources.categories
        if row.category_id in general_category_ids
        and row.name.casefold() == "advancement"
    ]
    if len(advancement_categories) != 1:
        raise ResourceError(
            "expected exactly one 'General / Advancement' achievement "
            f"category, found {len(advancement_categories)}"
        )
    advancement_category_ids = _category_descendants(
        resources, {advancement_categories[0]}
    )
    achievement_ids = {
        association.achievement_id
        for association in resources.category_associations
        if association.category_id in advancement_category_ids
    }
    achievements = {
        row.achievement_id: row for row in resources.achievements
    }

    selected: List[int] = []
    criteria: List[EvaluationCriterion] = []
    candidates = 0
    rejected = 0
    for achievement_id in sorted(achievement_ids):
        achievement = achievements.get(achievement_id)
        if achievement is None:
            continue
        name_match = re.fullmatch(
            r"([1-9][0-9]*) Alternate Advancement Points",
            achievement.name,
        )
        if name_match is None:
            continue
        spent_points = int(name_match.group(1), 10)
        if spent_points > max_spent:
            continue
        candidates += 1

        components = components_by_achievement.get(achievement_id, [])
        valid = (
            len(components) == 1
            and components[0].component_type == 1
            and components[0].sequence == 1
            and components[0].name
            == f"Spend {spent_points} Alternate Advancement Points"
            and achievement.description
            == (
                "This achievement is completed by spending "
                f"{spent_points} alternate advancement points."
            )
            and component_counts.get(components[0].component_id, 1) == 1
        )
        if not valid:
            rejected += 1
            continue

        component = components[0]
        selected.append(achievement_id)
        criteria.append(
            EvaluationCriterion(
                achievement_id=achievement_id,
                component_type=component.component_type,
                component_sequence=component.sequence,
                component_id=component.component_id,
                event_type=EVENT_ALTERNATE_ADVANCEMENT,
                progress_mode=PROGRESS_BOOLEAN,
                behavior=BEHAVIOR_REQUIRED,
                target_id=0,
                target_id2=0,
                target_value=spent_points,
            )
        )

    return tuple(selected), tuple(criteria), candidates, rejected


def _item_name_milestones(
    resources: ResourceSet,
    target_profile: Optional[ExpansionProfile],
    selected_ids: frozenset[int],
    components_by_achievement: dict[int, List[Component]],
    component_counts: dict[int, int],
) -> Tuple[
    Tuple[int, ...],
    Tuple[ItemNameCriterion, ...],
    int,
    int,
    int,
]:
    key_ids = (
        _general_category_achievement_ids(resources, "Keys")
        & selected_ids
    )
    class_ids = _general_category_achievement_ids(resources, "Class")
    achievements = {
        row.achievement_id: row for row in resources.achievements
    }
    presentation_components: dict[int, List[Component]] = {}
    for component in resources.components:
        if component.component_type == 3:
            presentation_components.setdefault(
                component.achievement_id, []
            ).append(component)

    target_index = (
        EXPANSION_PROFILES.index(target_profile)
        if target_profile is not None
        else -1
    )
    profile_indexes = {
        profile.key: index
        for index, profile in enumerate(EXPANSION_PROFILES)
    }
    epic_ids: set[int] = set()
    if target_profile is not None:
        for achievement_id in class_ids:
            achievement = achievements[achievement_id]
            version_match = re.fullmatch(
                r"Epic (1\.0|1\.5|2\.0)", achievement.name
            )
            description_match = re.fullmatch(
                r"This achievement is completed by obtaining the "
                r"(.+) Epic (1\.0|1\.5|2\.0)\.",
                achievement.description,
            )
            if (
                version_match is None
                or description_match is None
                or version_match.group(1) != description_match.group(2)
            ):
                continue
            class_name = description_match.group(1)
            if version_match.group(1) == "1.0":
                minimum_profile = {
                    "Beastlord": "luclin",
                    "Berserker": "god",
                }.get(class_name, "classic")
            else:
                minimum_profile = "oow"
            if target_index >= profile_indexes[minimum_profile]:
                epic_ids.add(achievement_id)

    candidates = 0
    rejected = 0
    selected_epics: List[int] = []
    selected_definitions = 0
    criteria: List[ItemNameCriterion] = []
    for achievement_id in sorted(key_ids | epic_ids):
        candidates += 1
        achievement = achievements[achievement_id]
        state_components = components_by_achievement.get(
            achievement_id, ()
        )
        is_epic = achievement_id in epic_ids
        valid = bool(state_components)
        definition_criteria: List[ItemNameCriterion] = []
        normalized_component_names: set[str] = set()
        required_class = 0

        if is_epic:
            version_match = re.fullmatch(
                r"Epic (1\.0|1\.5|2\.0)", achievement.name
            )
            description_match = re.fullmatch(
                r"This achievement is completed by obtaining the "
                r"(.+) Epic (1\.0|1\.5|2\.0)\.",
                achievement.description,
            )
            class_name = (
                description_match.group(1)
                if description_match is not None
                else ""
            )
            required_class = next(
                (
                    class_id
                    for mapped_name, class_id in CLASS_IDS.items()
                    if mapped_name.casefold() == class_name.casefold()
                ),
                0,
            )
            gates = presentation_components.get(achievement_id, ())
            valid = (
                valid
                and version_match is not None
                and description_match is not None
                and version_match.group(1) == description_match.group(2)
                and required_class != 0
                and len(gates) == 1
                and gates[0].component_id == 1302 + required_class
                and gates[0].name
                == (
                    "Meet or Exceed the Class Requirement "
                    f"({class_name})"
                )
            )

        for component in state_components:
            behavior = _criterion_behavior(component.component_type)
            if (
                behavior is None
                or component_counts.get(component.component_id, 1) != 1
            ):
                valid = False
                break

            item_names: List[str] = []
            if is_epic:
                item_match = re.fullmatch(r"Obtain (.+)", component.name)
                if item_match is None:
                    valid = False
                    break
                item_name = item_match.group(1)
                normalized_item_name = item_name.casefold()
                if normalized_item_name.startswith("the "):
                    item_name = item_name[4:]
                normalized_item_name = item_name.casefold()
                if normalized_item_name in normalized_component_names:
                    # Two state components with the same display name cannot be
                    # assigned safely to duplicate item IDs by an exact-name
                    # join (the Beastlord 1.0 pair is the known example).
                    valid = False
                    break
                normalized_component_names.add(normalized_item_name)
                item_names.append(item_name)
            else:
                alternatives = re.fullmatch(
                    r"(.+) or (.+)", component.name
                )
                if alternatives is not None:
                    item_names.extend(alternatives.groups())
                else:
                    item_names.append(component.name)
                if len(state_components) == 1:
                    item_names.append(achievement.name)

            for item_name in dict.fromkeys(item_names):
                if not item_name:
                    valid = False
                    break
                definition_criteria.append(
                    ItemNameCriterion(
                        achievement_id=achievement_id,
                        component_type=component.component_type,
                        component_sequence=component.sequence,
                        component_id=component.component_id,
                        behavior=behavior,
                        item_name=item_name,
                        required_class=required_class,
                    )
                )
            if not valid:
                break

        if not valid or not definition_criteria:
            rejected += 1
            continue
        selected_definitions += 1
        if is_epic:
            selected_epics.append(achievement_id)
        criteria.extend(definition_criteria)

    unique_criteria = {
        (
            row.achievement_id,
            row.component_type,
            row.component_sequence,
            row.component_id,
            row.behavior,
            row.item_name,
            row.required_class,
            row.required_count,
        ): row
        for row in criteria
    }
    return (
        tuple(selected_epics),
        tuple(
            sorted(
                unique_criteria.values(),
                key=lambda row: (
                    row.achievement_id,
                    row.component_type,
                    row.component_sequence,
                    row.component_id,
                    row.required_class,
                    row.item_name,
                ),
            )
        ),
        candidates,
        selected_definitions,
        rejected,
    )


def build_enable_selection(
    resources: ResourceSet,
    *,
    through_expansion: Optional[str] = None,
    max_level: Optional[int] = None,
    max_tradeskill_skill: Optional[int] = None,
    max_aa_spent: Optional[int] = None,
    enable_slayer: bool = False,
) -> EnableSelection:
    if (
        through_expansion is None
        and max_level is None
        and max_tradeskill_skill is None
        and max_aa_spent is None
        and not enable_slayer
    ):
        return EnableSelection(None, None, (), (), ())
    if max_level is not None and max_level < 1:
        raise ResourceError("max_level must be at least 1")
    if max_tradeskill_skill is not None and max_tradeskill_skill < 1:
        raise ResourceError("max_tradeskill_skill must be at least 1")
    if max_aa_spent is not None and max_aa_spent < 1:
        raise ResourceError("max_aa_spent must be at least 1")

    target_profile = (
        resolve_expansion(through_expansion)
        if through_expansion is not None
        else None
    )
    reviewed_profile_requested = (
        target_profile is not None
        and target_profile.key == "don"
    )
    if target_profile is not None and max_level is None:
        max_level = target_profile.level_cap

    achievement_ids = {row.achievement_id for row in resources.achievements}
    expansion_achievement_ids: set[int] = set()

    if target_profile is not None:
        target_index = EXPANSION_PROFILES.index(target_profile)
        selected_profiles = EXPANSION_PROFILES[: target_index + 1]
        root_ids: set[int] = set()
        for profile in selected_profiles:
            matches = [
                row.category_id
                for row in resources.categories
                if row.parent_id == 0
                and row.name.casefold()
                == profile.resource_category_name.casefold()
            ]
            if len(matches) != 1:
                raise ResourceError(
                    "expected exactly one top-level achievement category named "
                    f"{profile.resource_category_name!r}, found {len(matches)}"
                )
            root_ids.add(matches[0])

        expansion_category_ids = _category_descendants(resources, root_ids)
        expansion_achievement_ids = {
            row.achievement_id
            for row in resources.category_associations
            if row.category_id in expansion_category_ids
            and row.achievement_id in achievement_ids
        }

    components_by_achievement = _state_components_by_achievement(resources)
    component_counts = _required_counts(resources)

    level_achievement_ids: set[int] = set()
    level_criteria: List[LevelCriterion] = []
    if max_level is not None:
        general_roots = [
            row.category_id
            for row in resources.categories
            if row.parent_id == 0 and row.name.casefold() == "general"
        ]
        if len(general_roots) != 1:
            raise ResourceError(
                "expected exactly one top-level 'General' achievement category, "
                f"found {len(general_roots)}"
            )

        general_category_ids = _category_descendants(
            resources, {general_roots[0]}
        )
        level_categories = [
            row.category_id
            for row in resources.categories
            if row.category_id in general_category_ids
            and row.name.casefold() == "level"
        ]
        if len(level_categories) != 1:
            raise ResourceError(
                "expected exactly one 'General / Level' achievement category, "
                f"found {len(level_categories)}"
            )

        level_category_ids = _category_descendants(
            resources, {level_categories[0]}
        )
        associated_level_ids = {
            row.achievement_id
            for row in resources.category_associations
            if row.category_id in level_category_ids
            and row.achievement_id in achievement_ids
        }
        achievements_by_id = {
            row.achievement_id: row for row in resources.achievements
        }
        for achievement_id in associated_level_ids:
            achievement = achievements_by_id[achievement_id]
            match = re.fullmatch(
                r"Level\s+([1-9][0-9]*)",
                achievement.name,
                flags=re.IGNORECASE,
            )
            if match is None:
                raise ResourceError(
                    "cannot determine the level for achievement "
                    f"{achievement_id} ({achievement.name!r}) in the "
                    "'General / Level' category"
                )
            level = int(match.group(1), 10)
            if level > max_level:
                continue

            state_components = components_by_achievement.get(
                achievement_id, ()
            )
            expected_description = f"Reach Level {level}"
            if (
                achievement_id != level
                or len(state_components) != 1
                or state_components[0].component_type != 1
                or state_components[0].sequence != 1
                or state_components[0].component_id != level
                or state_components[0].name != expected_description
                or component_counts.get(state_components[0].component_id, 1) != 1
            ):
                raise ResourceError(
                    "cannot safely generate a Level criterion for achievement "
                    f"{achievement_id} ({achievement.name!r}); expected exactly one "
                    f"state-bearing component (sequence=1, type=1, component_id={level}, "
                    f"description={expected_description!r}) with required_count 1"
                )

            component = state_components[0]
            level_achievement_ids.add(achievement_id)
            level_criteria.append(
                LevelCriterion(
                    achievement_id=achievement_id,
                    component_type=component.component_type,
                    component_sequence=component.sequence,
                    component_id=component.component_id,
                    level=level,
                )
            )

    progression_level_achievement_ids: Tuple[int, ...] = ()
    progression_level_criteria: Tuple[LevelCriterion, ...] = ()
    progression_level_candidates = 0
    progression_level_rejected = 0
    skill_cap_achievement_ids: Tuple[int, ...] = ()
    skill_cap_criteria: Tuple[EvaluationCriterion, ...] = ()
    skill_cap_candidates = 0
    skill_cap_rejected = 0
    if max_level is not None:
        (
            progression_level_achievement_ids,
            progression_level_criteria,
            progression_level_candidates,
            progression_level_rejected,
        ) = _progression_level_milestones(
            resources,
            max_level,
            components_by_achievement,
            component_counts,
        )
        (
            skill_cap_achievement_ids,
            skill_cap_criteria,
            skill_cap_candidates,
            skill_cap_rejected,
        ) = _skill_cap_milestones(
            resources,
            max_level,
            components_by_achievement,
            component_counts,
        )

    if target_profile is not None and target_profile.key == "don":
        expansion_achievement_ids = _filter_don_historical_selection(
            resources,
            expansion_achievement_ids,
            components_by_achievement,
            component_counts,
        )

    tradeskill_achievement_ids: Tuple[int, ...] = ()
    tradeskill_criteria: Tuple[EvaluationCriterion, ...] = ()
    tradeskill_candidates = 0
    tradeskill_rejected = 0
    if max_tradeskill_skill is not None:
        (
            tradeskill_achievement_ids,
            tradeskill_criteria,
            tradeskill_candidates,
            tradeskill_rejected,
        ) = _tradeskill_milestones(
            resources,
            max_tradeskill_skill,
            components_by_achievement,
            component_counts,
        )

    aa_spent_achievement_ids: Tuple[int, ...] = ()
    aa_spent_criteria: Tuple[EvaluationCriterion, ...] = ()
    aa_spent_candidates = 0
    aa_spent_rejected = 0
    if max_aa_spent is not None:
        (
            aa_spent_achievement_ids,
            aa_spent_criteria,
            aa_spent_candidates,
            aa_spent_rejected,
        ) = _aa_spent_milestones(
            resources,
            max_aa_spent,
            components_by_achievement,
            component_counts,
        )

    slayer_achievement_ids: Tuple[int, ...] = ()
    slayer_criteria: Tuple[EvaluationCriterion, ...] = ()
    slayer_rejections: Tuple[str, ...] = ()
    slayer_candidates = 0
    slayer_direct_selected = 0
    slayer_meta_selected = 0
    if enable_slayer:
        (
            slayer_achievement_ids,
            slayer_criteria,
            slayer_rejections,
            slayer_candidates,
            slayer_direct_selected,
            slayer_meta_selected,
        ) = _slayer_criteria(
            resources,
            components_by_achievement,
            component_counts,
        )

    item_base_selected_ids = frozenset(
        expansion_achievement_ids
        | level_achievement_ids
        | set(progression_level_achievement_ids)
        | set(skill_cap_achievement_ids)
        | set(tradeskill_achievement_ids)
        | set(aa_spent_achievement_ids)
    )
    (
        item_achievement_ids,
        item_name_criteria,
        item_definition_candidates,
        item_definition_selected,
        item_definition_rejected,
    ) = _item_name_milestones(
        resources,
        target_profile,
        item_base_selected_ids,
        components_by_achievement,
        component_counts,
    )

    forced_disabled_achievement_ids = frozenset(
        REVIEWED_FORCED_DISABLED_ACHIEVEMENT_IDS & achievement_ids
        if reviewed_profile_requested
        else ()
    )
    reviewed_profile_achievement_ids = frozenset(
        (
            REVIEWED_PROGRESSION_PROFILE_IDS
            | REVIEWED_INVESTIGATION_ACHIEVEMENT_IDS
        )
        & achievement_ids
        if reviewed_profile_requested
        else ()
    )
    selected_ids = frozenset(
        expansion_achievement_ids
        | level_achievement_ids
        | set(progression_level_achievement_ids)
        | set(skill_cap_achievement_ids)
        | set(tradeskill_achievement_ids)
        | set(aa_spent_achievement_ids)
        | set(item_achievement_ids)
        | set(slayer_achievement_ids)
        | reviewed_profile_achievement_ids
    ) - forced_disabled_achievement_ids
    (
        traveler_criteria,
        traveler_locations,
        traveler_candidates,
        traveler_rejected,
    ) = _traveler_criteria(
        resources,
        selected_ids,
        components_by_achievement,
        component_counts,
    )
    (
        npc_name_criteria,
        npc_name_definitions,
        npc_name_unresolved_definitions,
        npc_name_rejected_components,
        npc_name_collision_components,
    ) = _npc_name_criteria(
        resources,
        selected_ids,
        traveler_locations,
        components_by_achievement,
        component_counts,
    )
    if reviewed_profile_requested:
        item_name_criteria = _without_reviewed_item_name_criteria(
            item_name_criteria
        )
    reviewed_criteria: Tuple[EvaluationCriterion, ...] = ()
    reviewed_script_criteria: Tuple[EvaluationCriterion, ...] = ()
    reviewed_source_component_keys: Tuple[Tuple[int, int, int], ...] = ()
    reviewed_script_component_keys: Tuple[Tuple[int, int, int], ...] = ()
    reviewed_presentation_component_keys: Tuple[Tuple[int, int, int], ...] = ()
    reviewed_unavailable_component_keys: Tuple[Tuple[int, int, int], ...] = ()
    reviewed_coverage_rejections: Tuple[str, ...] = ()
    if reviewed_profile_requested:
        (
            reviewed_criteria,
            reviewed_source_component_keys,
            reviewed_coverage_rejections,
        ) = _reviewed_source_criteria(
            resources,
            components_by_achievement,
            component_counts,
        )
        (
            reviewed_source_component_keys,
            reviewed_script_component_keys,
            reviewed_presentation_component_keys,
            reviewed_unavailable_component_keys,
        ) = _reviewed_coverage(
            resources, reviewed_source_component_keys
        )
        reviewed_script_criteria = _reviewed_script_criteria(
            components_by_achievement,
            component_counts,
            reviewed_script_component_keys,
        )
        expected_counts = (510, 214, 20, 330)
        actual_counts = (
            len(reviewed_source_component_keys),
            len(reviewed_script_component_keys),
            len(reviewed_presentation_component_keys),
            len(reviewed_unavailable_component_keys),
        )
        if actual_counts != expected_counts:
            reviewed_coverage_rejections = tuple(
                sorted(
                    {
                        *reviewed_coverage_rejections,
                        "reviewed DoN coverage count changed: expected "
                        f"{expected_counts}, found {actual_counts}",
                    }
                )
            )
    level_evaluation_criteria = tuple(
        _level_evaluation_criterion(criterion)
        for criterion in (
            *level_criteria,
            *progression_level_criteria,
        )
    )
    direct_criteria = (
        *level_evaluation_criteria,
        *skill_cap_criteria,
        *traveler_criteria,
        *tradeskill_criteria,
        *aa_spent_criteria,
        *npc_name_criteria,
        *slayer_criteria,
        *reviewed_criteria,
        *reviewed_script_criteria,
    )
    claimed_component_policies = _validated_component_policies(
        direct_criteria,
        item_name_criteria,
    )
    dependency_selected_ids = selected_ids
    if reviewed_profile_requested:
        dependency_selected_ids = selected_ids - (
            REVIEWED_INVESTIGATION_ACHIEVEMENT_IDS & achievement_ids
        )
    (
        dependency_criteria,
        superseded_criteria,
        dependency_ambiguous,
        dependency_self,
        dependency_rejected,
    ) = _achievement_dependency_criteria(
        resources,
        dependency_selected_ids,
        components_by_achievement,
        component_counts,
        claimed_component_policies,
    )
    generated_criteria = tuple(
        sorted(
            (
                *direct_criteria,
                *dependency_criteria,
            ),
            key=lambda criterion: (
                criterion.achievement_id,
                criterion.component_type,
                criterion.component_sequence,
                criterion.component_id,
                criterion.event_type,
                criterion.target_id,
                criterion.target_id2,
            ),
        )
    )
    _validated_component_policies(generated_criteria, item_name_criteria)
    _reject_dependency_cycles(
        tuple(
            criterion
            for criterion in generated_criteria
            if criterion.event_type == EVENT_ACHIEVEMENT_COMPLETE
        )
    )

    return EnableSelection(
        through_expansion=target_profile,
        max_level=max_level,
        expansion_achievement_ids=tuple(sorted(expansion_achievement_ids)),
        level_achievement_ids=tuple(sorted(level_achievement_ids)),
        level_criteria=tuple(
            sorted(level_criteria, key=lambda criterion: criterion.achievement_id)
        ),
        max_tradeskill_skill=max_tradeskill_skill,
        tradeskill_achievement_ids=tradeskill_achievement_ids,
        max_aa_spent=max_aa_spent,
        aa_spent_achievement_ids=aa_spent_achievement_ids,
        progression_level_achievement_ids=(
            progression_level_achievement_ids
        ),
        progression_level_criteria=progression_level_criteria,
        skill_cap_achievement_ids=skill_cap_achievement_ids,
        item_achievement_ids=item_achievement_ids,
        item_name_criteria=item_name_criteria,
        enable_slayer=enable_slayer,
        slayer_achievement_ids=slayer_achievement_ids,
        slayer_rejections=slayer_rejections,
        generated_criteria=generated_criteria,
        superseded_criteria=tuple(
            sorted(
                superseded_criteria,
                key=lambda criterion: (
                    criterion.achievement_id,
                    criterion.component_type,
                    criterion.component_sequence,
                    criterion.component_id,
                    criterion.event_type,
                    criterion.target_id,
                    criterion.target_id2,
                ),
            )
        ),
        reviewed_profile_achievement_ids=tuple(
            sorted(reviewed_profile_achievement_ids)
        ),
        forced_disabled_achievement_ids=tuple(
            sorted(forced_disabled_achievement_ids)
        ),
        reviewed_source_component_keys=reviewed_source_component_keys,
        reviewed_script_component_keys=reviewed_script_component_keys,
        reviewed_presentation_component_keys=(
            reviewed_presentation_component_keys
        ),
        reviewed_unavailable_component_keys=(
            reviewed_unavailable_component_keys
        ),
        reviewed_coverage_rejections=reviewed_coverage_rejections,
        criteria_report=CriteriaReport(
            progression_level_candidates=(
                progression_level_candidates
            ),
            progression_level_generated=len(
                progression_level_criteria
            ),
            progression_level_rejected=(
                progression_level_rejected
            ),
            skill_cap_candidates=skill_cap_candidates,
            skill_cap_generated=len(skill_cap_criteria),
            skill_cap_rejected=skill_cap_rejected,
            item_definition_candidates=item_definition_candidates,
            item_definition_selected=item_definition_selected,
            item_definition_rejected=item_definition_rejected,
            item_name_mappings=len(item_name_criteria),
            traveler_candidates=traveler_candidates,
            traveler_generated=len(traveler_criteria),
            traveler_rejected=traveler_rejected,
            dependency_generated=len(dependency_criteria),
            dependency_superseded=len(superseded_criteria),
            dependency_ambiguous=dependency_ambiguous,
            dependency_self=dependency_self,
            dependency_rejected=dependency_rejected,
            tradeskill_candidates=tradeskill_candidates,
            tradeskill_generated=len(tradeskill_criteria),
            tradeskill_rejected=tradeskill_rejected,
            aa_spent_candidates=aa_spent_candidates,
            aa_spent_generated=len(aa_spent_criteria),
            aa_spent_rejected=aa_spent_rejected,
            npc_name_definitions=npc_name_definitions,
            npc_name_unresolved_definitions=(
                npc_name_unresolved_definitions
            ),
            npc_name_generated=len(npc_name_criteria),
            npc_name_rejected_components=(
                npc_name_rejected_components
            ),
            npc_name_collision_components=(
                npc_name_collision_components
            ),
            slayer_candidates=slayer_candidates,
            slayer_direct_selected=slayer_direct_selected,
            slayer_meta_selected=slayer_meta_selected,
            slayer_generated=len(slayer_criteria),
            slayer_rejected=len(slayer_rejections),
            reviewed_source_components=len(
                reviewed_source_component_keys
            ),
            reviewed_script_components=len(
                reviewed_script_component_keys
            ),
            reviewed_presentation_components=len(
                reviewed_presentation_component_keys
            ),
            reviewed_unavailable_components=len(
                reviewed_unavailable_component_keys
            ),
        ),
    )


def _sql_text(value: str) -> str:
    if value == "":
        return "''"

    # Hex encoding makes generated SQL independent of quote and backslash SQL
    # modes while preserving the source UTF-8 bytes exactly.
    encoded = value.encode("utf-8", errors="strict").hex().upper()
    return f"CONVERT(0x{encoded} USING utf8mb4)"


def _emit_insert(
    output: List[str],
    *,
    table: str,
    columns: Sequence[str],
    rows: Sequence[object],
    values: Callable[[object], Sequence[str]],
    updates: Sequence[str],
) -> None:
    quoted_columns = ", ".join(f"`{column}`" for column in columns)
    for start in range(0, len(rows), INSERT_BATCH_SIZE):
        batch = rows[start : start + INSERT_BATCH_SIZE]
        output.append(f"INSERT INTO `{table}` ({quoted_columns}) VALUES")
        for index, row in enumerate(batch):
            suffix = "," if index + 1 < len(batch) else ""
            output.append(f"({', '.join(values(row))}){suffix}")
        output.append("ON DUPLICATE KEY UPDATE")
        output.append(
            ",\n".join(
                f"\t`{column}` = VALUES(`{column}`)" for column in updates
            )
            + ";"
        )
        output.append("")


def _emit_key_rows(
    output: List[str],
    *,
    table: str,
    columns: Sequence[str],
    rows: Sequence[Sequence[int]],
) -> None:
    if not rows:
        return

    quoted_columns = ", ".join(f"`{column}`" for column in columns)
    for start in range(0, len(rows), INSERT_BATCH_SIZE):
        batch = rows[start : start + INSERT_BATCH_SIZE]
        output.append(f"INSERT IGNORE INTO `{table}` ({quoted_columns}) VALUES")
        for index, row in enumerate(batch):
            suffix = "," if index + 1 < len(batch) else ";"
            output.append(f"({', '.join(str(value) for value in row)}){suffix}")
        output.append("")


def _emit_enable_updates(output: List[str], achievement_ids: Sequence[int]) -> None:
    if not achievement_ids:
        return

    output.extend(
        (
            "-- Enable only the definitions selected by progression options.",
            "-- Definitions outside the selection retain their existing enabled state.",
        )
    )
    for start in range(0, len(achievement_ids), INSERT_BATCH_SIZE):
        batch = achievement_ids[start : start + INSERT_BATCH_SIZE]
        output.append(
            "UPDATE `achievements` SET `enabled` = 1 WHERE `id` IN ("
            + ", ".join(str(value) for value in batch)
            + ");"
        )
    output.append("")


def _emit_exact_disable_updates(
    output: List[str], achievement_ids: Sequence[int]
) -> None:
    if not achievement_ids:
        return

    output.extend(
        (
            "-- Exact enable selection requested: disable only unselected IDs",
            "-- present in this validated resource snapshot. Custom IDs are untouched.",
        )
    )
    for start in range(0, len(achievement_ids), INSERT_BATCH_SIZE):
        batch = achievement_ids[start : start + INSERT_BATCH_SIZE]
        output.append(
            "UPDATE `achievements` SET `enabled` = 0 WHERE `id` IN ("
            + ", ".join(str(value) for value in batch)
            + ");"
        )
    output.append("")


def _achievement_category_paths(
    resources: ResourceSet,
) -> dict[int, Tuple[str, ...]]:
    categories = {row.category_id: row for row in resources.categories}
    paths: dict[int, str] = {}

    def category_path(category_id: int) -> str:
        cached = paths.get(category_id)
        if cached is not None:
            return cached

        names: List[str] = []
        seen: set[int] = set()
        current_id = category_id
        while current_id != 0 and current_id not in seen:
            seen.add(current_id)
            current = categories.get(current_id)
            if current is None:
                names.append(f"category {current_id}")
                break
            names.append(current.name or f"category {current_id}")
            current_id = current.parent_id
        path = " / ".join(reversed(names)) or "unassociated"
        paths[category_id] = path
        return path

    associations: dict[int, set[str]] = {}
    for row in resources.category_associations:
        associations.setdefault(row.achievement_id, set()).add(
            category_path(row.category_id)
        )
    return {
        achievement_id: tuple(sorted(values))
        for achievement_id, values in associations.items()
    }


def _reviewed_unavailable_reason(achievement_id: int) -> str:
    for reason, achievement_ids in REVIEWED_UNAVAILABLE_DEFINITION_GROUPS:
        if achievement_id in achievement_ids:
            return reason
    return "reviewed definition is not completable in the audited server profile"


def _comment_field(value: str) -> str:
    return " ".join(value.split())


def _emit_reviewed_disable_updates(
    output: List[str],
    resources: ResourceSet,
    achievement_ids: Sequence[int],
) -> None:
    if not achievement_ids:
        return

    achievements = {
        row.achievement_id: row for row in resources.achievements
    }
    category_paths = _achievement_category_paths(resources)
    output.extend(
        (
            "-- Reviewed RoF2/DoN exclusions are always disabled, including when",
            "-- --preserve-enable-state is used. These definitions cannot currently",
            "-- be completed from authoritative server or quest state.",
        )
    )
    for achievement_id in achievement_ids:
        achievement = achievements.get(achievement_id)
        name = (
            _comment_field(achievement.name)
            if achievement is not None
            else "missing resource definition"
        )
        categories = "; ".join(
            category_paths.get(achievement_id, ("unassociated",))
        )
        output.append(
            f"-- Disabled {achievement_id}: {name} | "
            f"{_comment_field(categories)} | "
            f"{_reviewed_unavailable_reason(achievement_id)}"
        )
    for start in range(0, len(achievement_ids), INSERT_BATCH_SIZE):
        batch = achievement_ids[start : start + INSERT_BATCH_SIZE]
        output.append(
            "UPDATE `achievements` SET `enabled` = 0 WHERE `id` IN ("
            + ", ".join(str(value) for value in batch)
            + ");"
        )
    output.append("")


def _emit_component_identity_lines(
    output: List[str],
    label: str,
    component_keys: Sequence[Tuple[int, int, int]],
) -> None:
    if not component_keys:
        output.append(f"-- {label}: none")
        return
    values = [
        f"{achievement_id}/{component_type}/{component_id}"
        for achievement_id, component_type, component_id in component_keys
    ]
    for start in range(0, len(values), 8):
        prefix = f"-- {label}: " if start == 0 else f"-- {label}+: "
        output.append(prefix + ", ".join(values[start : start + 8]))


def _emit_reviewed_coverage_report(
    output: List[str],
    resources: ResourceSet,
    selection: EnableSelection,
) -> None:
    if not (
        selection.reviewed_source_component_keys
        or selection.reviewed_script_component_keys
        or selection.reviewed_presentation_component_keys
        or selection.reviewed_unavailable_component_keys
        or selection.forced_disabled_achievement_ids
    ):
        return

    output.extend(
        (
            "-- Reviewed achievement coverage audit",
            "-- Scope: the supplied ToB achievement resource snapshot, evaluated for",
            "-- native RoF2 server behavior and launch-through-Dragons of Norrath",
            "-- content at level 70. Later client definitions are outside this profile.",
            "-- Source-native components have deterministic server criteria. Quest-manual",
            "-- components require an authoritative script hook. Presentation components",
            "-- only group or label state and must not be treated as completion facts.",
            "-- Unavailable components depend on absent content, entities, or hooks.",
            "-- Investigation definitions may remain enabled; remaining reviewed definitions",
            "-- listed below are explicitly disabled.",
            f"-- Coverage totals: {len(selection.reviewed_source_component_keys)} "
            "source-native; "
            f"{len(selection.reviewed_script_component_keys)} quest-manual; "
            f"{len(selection.reviewed_presentation_component_keys)} presentation-only; "
            f"{len(selection.reviewed_unavailable_component_keys)} unavailable.",
        )
    )
    _emit_component_identity_lines(
        output,
        "source-native",
        selection.reviewed_source_component_keys,
    )
    _emit_component_identity_lines(
        output,
        "quest-manual",
        selection.reviewed_script_component_keys,
    )
    _emit_component_identity_lines(
        output,
        "presentation-only",
        selection.reviewed_presentation_component_keys,
    )
    _emit_component_identity_lines(
        output,
        "unavailable",
        selection.reviewed_unavailable_component_keys,
    )
    output.extend(
        (
            "-- City achievement audit: client IDs 12000-12143 contain 143 definitions",
            "-- (12080 is absent). Quest review found 122 hooks, but 12016's NPC is",
            "-- unspawned; 121 are currently playable and 22 remain enabled for investigation.",
            "-- Quest hook contract: update a step only after its authoritative success",
            "-- condition with non-additive SetAchievementProgress(achievement_id,",
            "-- component_type, component_id, 1). CompleteAchievement is reserved for",
            "-- a verified whole-achievement grant. Group, raid, DZ, and task participants",
            "-- in other zones must be updated through the world state-update path.",
            "-- Reconciliation contract: durable TaskComplete, OwnItem, and dependency",
            "-- facts are reconciled by server criteria. Script-only facts must also be",
            "-- persisted by their owning quest/task system so login reconciliation can",
            "-- restore retroactive completion without replaying one-time rewards.",
        )
    )
    for rejection in selection.reviewed_coverage_rejections:
        output.append(
            f"-- REVIEWED COVERAGE REJECTION: {_comment_field(rejection)}"
        )
    output.append("")


def _emit_superseded_criteria(
    output: List[str], criteria: Sequence[EvaluationCriterion]
) -> None:
    identities = tuple(
        sorted(
            {
                (
                    criterion.achievement_id,
                    criterion.component_type,
                    criterion.component_id,
                    criterion.event_type,
                    criterion.target_id,
                    criterion.target_id2,
                )
                for criterion in criteria
            }
        )
    )
    if not identities:
        return

    output.extend(
        (
            "-- Disable exact criterion identities emitted by older importer "
            "inference that are superseded by direct event mappings.",
            "-- Rows are retained for audit/recovery and no unrelated criteria "
            "are changed.",
        )
    )
    columns = (
        "`achievement_id`, `component_type`, `component_id`, "
        "`event_type`, `target_id`, `target_id2`"
    )
    for start in range(0, len(identities), INSERT_BATCH_SIZE):
        batch = identities[start : start + INSERT_BATCH_SIZE]
        output.extend(
            (
                "UPDATE `achievement_criteria`",
                "SET `enabled` = 0",
                f"WHERE ({columns}) IN (",
            )
        )
        for index, identity in enumerate(batch):
            suffix = "," if index + 1 < len(batch) else ""
            output.append(
                "\t(" + ", ".join(str(value) for value in identity) + ")" + suffix
            )
        output.extend((");", ""))


def _emit_reviewed_own_item_reset(
    output: List[str],
    component_keys: Sequence[Tuple[int, int, int]],
) -> None:
    reviewed_keys = tuple(
        sorted(set(component_keys) & set(REVIEWED_OWN_ITEM_TARGETS))
    )
    if not reviewed_keys:
        return

    output.extend(
        (
            "-- Replace older item-name inference for reviewed ownership components",
            "-- with the exact numeric item alternatives emitted below.",
        )
    )
    for start in range(0, len(reviewed_keys), INSERT_BATCH_SIZE):
        batch = reviewed_keys[start : start + INSERT_BATCH_SIZE]
        identities = ", ".join(
            f"({achievement_id}, {component_type}, {component_id})"
            for achievement_id, component_type, component_id in batch
        )
        output.extend(
            (
                "UPDATE `achievement_criteria` SET `enabled` = 0",
                f"WHERE `event_type` = {EVENT_OWN_ITEM}",
                "AND (`achievement_id`, `component_type`, `component_id`) IN "
                f"({identities});",
            )
        )
    output.append("")


def _emit_criteria(
    output: List[str], criteria: Sequence[EvaluationCriterion]
) -> None:
    if not criteria:
        return

    output.extend(
        (
            "-- Generate only structurally validated evaluation criteria.",
            "-- Existing criteria with other identities remain untouched.",
        )
    )
    _emit_insert(
        output,
        table="achievement_criteria",
        columns=(
            "achievement_id",
            "component_type",
            "component_sequence",
            "component_id",
            "event_type",
            "progress_mode",
            "behavior",
            "target_id",
            "target_id2",
            "target_value",
            "required_count",
            "enabled",
        ),
        rows=criteria,
        values=lambda value: (
            str(value.achievement_id),
            str(value.component_type),
            str(value.component_sequence),
            str(value.component_id),
            str(value.event_type),
            str(value.progress_mode),
            str(value.behavior),
            str(value.target_id),
            str(value.target_id2),
            str(value.target_value),
            str(value.required_count),
            "1",
        ),
        updates=(
            "component_sequence",
            "progress_mode",
            "behavior",
            "target_value",
            "required_count",
            "enabled",
        ),
    )


def _emit_item_name_criteria(
    output: List[str], criteria: Sequence[ItemNameCriterion]
) -> None:
    if not criteria:
        return

    output.extend(
        (
            "-- Resolve structurally validated item components against this "
            "server's item table.",
            "-- Every exact-name duplicate item ID is an OR alternative for "
            "the same component.",
        )
    )
    for start in range(0, len(criteria), INSERT_BATCH_SIZE):
        batch = criteria[start : start + INSERT_BATCH_SIZE]
        output.extend(
            (
                "INSERT INTO `achievement_criteria` (",
                "\t`achievement_id`, `component_type`, "
                "`component_sequence`, `component_id`,",
                "\t`event_type`, `progress_mode`, `behavior`, `target_id`, "
                "`target_id2`,",
                "\t`target_value`, `required_count`, `enabled`",
                ")",
                "SELECT",
                "\tmapping.`achievement_id`, mapping.`component_type`,",
                "\tmapping.`component_sequence`, mapping.`component_id`,",
                f"\t{EVENT_OWN_ITEM}, {PROGRESS_BOOLEAN}, "
                "mapping.`behavior`, matched_item.`id`, "
                "mapping.`required_class`,",
                "\t1, mapping.`required_count`, 1",
                "FROM (",
            )
        )
        for index, row in enumerate(batch):
            prefix = "\tSELECT " if index == 0 else "\tUNION ALL SELECT "
            output.append(
                prefix
                + ", ".join(
                    (
                        f"{row.achievement_id} AS `achievement_id`",
                        f"{row.component_type} AS `component_type`",
                        f"{row.component_sequence} AS `component_sequence`",
                        f"{row.component_id} AS `component_id`",
                        f"{row.behavior} AS `behavior`",
                        f"{row.required_class} AS `required_class`",
                        f"{row.required_count} AS `required_count`",
                        f"{_sql_text(row.item_name)} AS `item_name`",
                    )
                )
            )
        output.extend(
            (
                ") AS mapping",
                "INNER JOIN `items` AS matched_item",
                "\tON LOWER(matched_item.`Name`) = "
                "LOWER(mapping.`item_name`)",
                "ON DUPLICATE KEY UPDATE",
                "\t`component_sequence` = "
                "VALUES(`component_sequence`),",
                "\t`progress_mode` = VALUES(`progress_mode`),",
                "\t`behavior` = VALUES(`behavior`),",
                "\t`target_value` = VALUES(`target_value`),",
                "\t`required_count` = VALUES(`required_count`),",
                "\t`enabled` = VALUES(`enabled`);",
                "",
            )
        )


def generate_sql(
    resources: ResourceSet,
    replace_existing: bool = False,
    enable_selection: Optional[EnableSelection] = None,
    exact_enable_selection: bool = False,
    preserve_enable_state: bool = False,
) -> str:
    enabled_ids = (
        enable_selection.achievement_ids
        if enable_selection is not None
        else frozenset()
    )
    selection_requested = (
        enable_selection is not None
        and (
            enable_selection.through_expansion is not None
            or enable_selection.max_level is not None
            or enable_selection.max_tradeskill_skill is not None
            or enable_selection.max_aa_spent is not None
            or enable_selection.enable_slayer
        )
    )
    if exact_enable_selection and not selection_requested:
        raise ResourceError(
            "exact_enable_selection requires at least one progression "
            "selection option"
        )
    if exact_enable_selection and replace_existing:
        raise ResourceError(
            "exact_enable_selection cannot be combined with replace_existing; "
            "replacement removes custom definitions absent from the resource "
            "snapshot"
        )
    if preserve_enable_state and not selection_requested:
        raise ResourceError(
            "preserve_enable_state requires at least one progression "
            "selection option"
        )
    if preserve_enable_state and exact_enable_selection:
        raise ResourceError(
            "preserve_enable_state cannot be combined with "
            "exact_enable_selection"
        )
    generated_criteria = (
        enable_selection.generated_criteria
        if enable_selection is not None
        else ()
    )
    item_name_criteria = (
        enable_selection.item_name_criteria
        if enable_selection is not None
        else ()
    )
    superseded_criteria = (
        enable_selection.superseded_criteria
        if enable_selection is not None
        else ()
    )
    output = [
        "-- Generated by utils/scripts/import_achievement_resources.py.",
        "-- AchievementsClient field 6 indicates whether a reward is shown;",
        "-- field 7 is preserved as client_flag without assigning semantics.",
        "-- RoF2's pre-component fields are persistent=1 and the server-authored",
        "-- version; other runtime policy remains server-authored.",
    ]
    if selection_requested:
        if preserve_enable_state:
            output.extend(
                (
                    "-- Progression options select generated criteria and the "
                    "enabled state for newly inserted definitions;",
                    "-- existing definitions retain their current enabled state.",
                )
            )
        elif exact_enable_selection:
            output.extend(
                (
                    "-- Definitions selected by progression options are enabled;",
                    "-- other imported snapshot definitions are disabled, while custom IDs",
                    "-- absent from the snapshot retain their existing state.",
                )
            )
        else:
            output.extend(
                (
                    "-- Definitions selected by progression options are inserted/enabled;",
                    "-- other new definitions are disabled and existing definitions keep their state.",
                )
            )
    else:
        output.extend(
            (
                "-- Newly inserted definitions are disabled;",
                "-- existing definitions keep their current enabled state.",
            )
        )
    output.append(
        f"-- Rows: {len(resources.categories)} categories, "
        f"{len(resources.achievements)} achievements, "
        f"{len(resources.category_associations)} category associations, "
        f"{len(resources.components)} components, "
        f"{len(resources.component_counts)} component counts."
    )
    if selection_requested:
        assert enable_selection is not None
        report = enable_selection.criteria_report
        output.extend(
            (
                f"-- Generated {len(generated_criteria)} fixed validated criteria "
                f"plus {len(item_name_criteria)} exact-item-name mappings: "
                f"{len(enable_selection.level_criteria)} level, "
                f"{report.progression_level_generated} progression-level, "
                f"{report.skill_cap_generated} skill-cap, "
                f"{report.item_name_mappings} item-name, "
                f"{report.traveler_generated} traveler, "
                f"{report.dependency_generated} dependency, "
                f"{report.tradeskill_generated} tradeskill, "
                f"{report.aa_spent_generated} AA-spent, "
                f"{report.npc_name_generated} named-kill, "
                f"{report.slayer_generated} Slayer.",
                f"-- Candidates: {report.progression_level_candidates} "
                "progression-level definitions, "
                f"{report.skill_cap_candidates} skill-cap definitions, "
                f"{report.item_definition_candidates} item definitions, "
                f"{report.traveler_candidates} traveler "
                "definitions, "
                f"{report.tradeskill_candidates} direct tradeskill "
                "milestones, "
                f"{report.aa_spent_candidates} AA-spent milestones, "
                f"{report.npc_name_definitions} Hunter definitions, "
                f"{report.slayer_candidates} Slayer definitions.",
                f"-- Skipped/rejected: "
                f"{report.progression_level_rejected} progression-level, "
                f"{report.skill_cap_rejected} skill-cap, "
                f"{report.item_definition_rejected} item definitions, "
                f"{report.traveler_rejected} traveler, "
                f"{report.dependency_superseded} superseded dependency, "
                f"{report.dependency_ambiguous} ambiguous dependency, "
                f"{report.dependency_self} self dependency, "
                f"{report.dependency_rejected} invalid dependency, "
                f"{report.tradeskill_rejected} tradeskill, "
                f"{report.aa_spent_rejected} AA-spent, "
                f"{report.npc_name_unresolved_definitions} unresolved "
                "named-kill definitions, "
                f"{report.npc_name_rejected_components} named-kill "
                "components, "
                f"{report.npc_name_collision_components} named-kill "
                f"hash collisions, {report.slayer_rejected} Slayer "
                "definitions.",
                "-- Criteria with other identities, rewards, cast restrictions, "
                "and character state are untouched.",
            )
        )
        for rejection in enable_selection.slayer_rejections:
            output.append(f"-- Slayer rejection: {rejection}")
    else:
        output.append(
            "-- Server criteria, rewards, cast restrictions, and character state are untouched."
        )
    if selection_requested:
        assert enable_selection is not None
        expansion_text = (
            enable_selection.through_expansion.resource_category_name
            if enable_selection.through_expansion is not None
            else "none"
        )
        level_text = (
            str(enable_selection.max_level)
            if enable_selection.max_level is not None
            else "none"
        )
        tradeskill_text = (
            str(enable_selection.max_tradeskill_skill)
            if enable_selection.max_tradeskill_skill is not None
            else "none"
        )
        aa_spent_text = (
            str(enable_selection.max_aa_spent)
            if enable_selection.max_aa_spent is not None
            else "none"
        )
        output.extend(
            (
                f"-- Enable selection: launch through {expansion_text}; "
                f"level milestones through {level_text}; "
                f"tradeskill milestones through {tradeskill_text}; "
                f"AA-spent milestones through {aa_spent_text}; "
                f"Slayer {'requested' if enable_selection.enable_slayer else 'not selected'}.",
                (
                    f"-- Selected {len(enabled_ids)} definitions "
                    f"({len(enable_selection.expansion_achievement_ids)} expansion, "
                    f"{len(enable_selection.level_achievement_ids)} level, "
                    f"{len(enable_selection.progression_level_achievement_ids)} "
                    "progression-level, "
                    f"{len(enable_selection.skill_cap_achievement_ids)} "
                    "skill-cap, "
                    f"{len(enable_selection.item_achievement_ids)} "
                    "additional item, "
                    f"{len(enable_selection.tradeskill_achievement_ids)} "
                    "tradeskill, "
                    f"{len(enable_selection.aa_spent_achievement_ids)} "
                    "AA-spent, "
                    f"{len(enable_selection.slayer_achievement_ids)} "
                    "Slayer, "
                    f"{len(enable_selection.reviewed_profile_achievement_ids)} "
                    "reviewed progression achievements)."
                ),
            )
        )
        if exact_enable_selection:
            output.append(
                "-- Exact enable selection disables other imported snapshot "
                "definitions before enabling this selection."
            )
        _emit_reviewed_coverage_report(output, resources, enable_selection)
    output.extend(("", "SET NAMES utf8mb4;", "START TRANSACTION;", ""))

    if replace_existing:
        output.extend(
            [
                "-- Exact-snapshot mode requested by --replace-existing.",
                "-- Temporary key sets remove stale presentation rows after upserts",
                "-- without resetting server-authored fields on surviving rows.",
                "DROP TEMPORARY TABLE IF EXISTS `_achievement_import_categories`;",
                "DROP TEMPORARY TABLE IF EXISTS `_achievement_import_definitions`;",
                "DROP TEMPORARY TABLE IF EXISTS `_achievement_import_category_links`;",
                "DROP TEMPORARY TABLE IF EXISTS `_achievement_import_components`;",
                "DROP TEMPORARY TABLE IF EXISTS `_achievement_import_counts`;",
                "CREATE TEMPORARY TABLE `_achievement_import_categories` (",
                "\t`id` INT(10) UNSIGNED NOT NULL, PRIMARY KEY (`id`)",
                ") ENGINE=InnoDB;",
                "CREATE TEMPORARY TABLE `_achievement_import_definitions` (",
                "\t`id` INT(10) UNSIGNED NOT NULL, PRIMARY KEY (`id`)",
                ") ENGINE=InnoDB;",
                "CREATE TEMPORARY TABLE `_achievement_import_category_links` (",
                "\t`category_id` INT(10) UNSIGNED NOT NULL,",
                "\t`achievement_id` INT(10) UNSIGNED NOT NULL,",
                "\tPRIMARY KEY (`category_id`, `achievement_id`)",
                ") ENGINE=InnoDB;",
                "CREATE TEMPORARY TABLE `_achievement_import_components` (",
                "\t`achievement_id` INT(10) UNSIGNED NOT NULL,",
                "\t`component_type` TINYINT(3) UNSIGNED NOT NULL,",
                "\t`component_id` INT(10) UNSIGNED NOT NULL,",
                "\tPRIMARY KEY (`achievement_id`, `component_type`, `component_id`)",
                ") ENGINE=InnoDB;",
                "CREATE TEMPORARY TABLE `_achievement_import_counts` (",
                "\t`component_id` INT(10) UNSIGNED NOT NULL, PRIMARY KEY (`component_id`)",
                ") ENGINE=InnoDB;",
                "",
            ]
        )
        _emit_key_rows(
            output,
            table="_achievement_import_categories",
            columns=("id",),
            rows=tuple((row.category_id,) for row in resources.categories),
        )
        _emit_key_rows(
            output,
            table="_achievement_import_definitions",
            columns=("id",),
            rows=tuple((row.achievement_id,) for row in resources.achievements),
        )
        _emit_key_rows(
            output,
            table="_achievement_import_category_links",
            columns=("category_id", "achievement_id"),
            rows=tuple(
                (row.category_id, row.achievement_id)
                for row in resources.category_associations
            ),
        )
        _emit_key_rows(
            output,
            table="_achievement_import_components",
            columns=("achievement_id", "component_type", "component_id"),
            rows=tuple(
                (row.achievement_id, row.component_type, row.component_id)
                for row in resources.components
            ),
        )
        _emit_key_rows(
            output,
            table="_achievement_import_counts",
            columns=("component_id",),
            rows=tuple((row.component_id,) for row in resources.component_counts),
        )

    _emit_insert(
        output,
        table="achievement_categories",
        columns=("id", "parent_id", "sequence", "name", "description", "icon"),
        rows=resources.categories,
        values=lambda value: (
            str(value.category_id),
            str(value.parent_id),
            str(value.sequence),
            _sql_text(value.name),
            _sql_text(value.description),
            _sql_text(value.icon),
        ),
        updates=("parent_id", "sequence", "name", "description", "icon"),
    )

    _emit_insert(
        output,
        table="achievements",
        columns=(
            "id",
            "name",
            "description",
            "icon_id",
            "points",
            "has_reward",
            "client_flag",
            "enabled",
        ),
        rows=resources.achievements,
        values=lambda value: (
            str(value.achievement_id),
            _sql_text(value.name),
            _sql_text(value.description),
            str(value.icon_id),
            str(value.points),
            "1" if value.has_reward else "0",
            str(value.client_flag),
            "1" if value.achievement_id in enabled_ids else "0",
        ),
        updates=(
            "name",
            "description",
            "icon_id",
            "points",
            "has_reward",
            "client_flag",
        ),
    )

    if exact_enable_selection:
        _emit_exact_disable_updates(
            output,
            tuple(
                sorted(
                    {
                        row.achievement_id
                        for row in resources.achievements
                    }
                    - enabled_ids
                )
            ),
        )
    if not preserve_enable_state:
        _emit_enable_updates(output, tuple(sorted(enabled_ids)))
    if enable_selection is not None:
        _emit_reviewed_disable_updates(
            output,
            resources,
            enable_selection.forced_disabled_achievement_ids,
        )

    _emit_insert(
        output,
        table="achievement_category_associations",
        columns=("category_id", "sequence", "achievement_id"),
        rows=resources.category_associations,
        values=lambda value: (
            str(value.category_id),
            str(value.sequence),
            str(value.achievement_id),
        ),
        updates=("sequence",),
    )

    _emit_insert(
        output,
        table="achievement_components",
        columns=(
            "achievement_id",
            "component_type",
            "sequence",
            "component_id",
            "name",
            "description",
        ),
        rows=resources.components,
        values=lambda value: (
            str(value.achievement_id),
            str(value.component_type),
            str(value.sequence),
            str(value.component_id),
            _sql_text(value.name),
            "''",
        ),
        updates=("sequence", "name"),
    )

    _emit_insert(
        output,
        table="achievement_associations",
        columns=("component_id", "required_count"),
        rows=resources.component_counts,
        values=lambda value: (
            str(value.component_id),
            str(value.required_count),
        ),
        updates=("required_count",),
    )

    if enable_selection is not None:
        _emit_reviewed_own_item_reset(
            output, enable_selection.reviewed_source_component_keys
        )
    _emit_superseded_criteria(output, superseded_criteria)
    _emit_criteria(output, generated_criteria)
    _emit_item_name_criteria(output, item_name_criteria)

    if replace_existing:
        output.extend(
            (
                "-- Remove only presentation rows absent from the validated snapshot.",
                "DELETE target FROM `achievement_category_associations` AS target",
                "LEFT JOIN `_achievement_import_category_links` AS source",
                "\tON source.`category_id` = target.`category_id`",
                "\tAND source.`achievement_id` = target.`achievement_id`",
                "WHERE source.`category_id` IS NULL;",
                "",
                "DELETE target FROM `achievement_associations` AS target",
                "LEFT JOIN `_achievement_import_counts` AS source",
                "\tON source.`component_id` = target.`component_id`",
                "WHERE source.`component_id` IS NULL;",
                "",
                "DELETE target FROM `achievement_components` AS target",
                "LEFT JOIN `_achievement_import_components` AS source",
                "\tON source.`achievement_id` = target.`achievement_id`",
                "\tAND source.`component_type` = target.`component_type`",
                "\tAND source.`component_id` = target.`component_id`",
                "WHERE source.`achievement_id` IS NULL;",
                "",
                "DELETE target FROM `achievement_categories` AS target",
                "LEFT JOIN `_achievement_import_categories` AS source",
                "\tON source.`id` = target.`id`",
                "WHERE source.`id` IS NULL;",
                "",
                "DELETE target FROM `achievements` AS target",
                "LEFT JOIN `_achievement_import_definitions` AS source",
                "\tON source.`id` = target.`id`",
                "WHERE source.`id` IS NULL;",
                "",
                "DROP TEMPORARY TABLE `_achievement_import_counts`;",
                "DROP TEMPORARY TABLE `_achievement_import_components`;",
                "DROP TEMPORARY TABLE `_achievement_import_category_links`;",
                "DROP TEMPORARY TABLE `_achievement_import_definitions`;",
                "DROP TEMPORARY TABLE `_achievement_import_categories`;",
                "",
            )
        )

    output.extend(("COMMIT;", ""))
    return "\n".join(output)


def _write_output(destination: str, sql: str) -> None:
    if destination == "-":
        sys.stdout.write(sql)
        return

    output_path = Path(destination).expanduser()
    if not output_path.parent.is_dir():
        raise ResourceError(f"output directory does not exist: {output_path.parent}")

    temporary_path = output_path.with_name(
        f".{output_path.name}.{os.getpid()}.temporary"
    )
    try:
        with temporary_path.open(
            "w", encoding="utf-8", errors="strict", newline="\n"
        ) as output:
            output.write(sql)
        os.replace(temporary_path, output_path)
    except OSError as error:
        raise ResourceError(f"unable to write {output_path}: {error}") from error
    finally:
        try:
            temporary_path.unlink()
        except FileNotFoundError:
            pass


def _build_argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Validate the five client achievement resource files and emit SQL "
            "for their UI definition tables."
        )
    )
    parser.add_argument(
        "resource_directory",
        type=Path,
        help="directory containing the five achievement resource text files",
    )
    parser.add_argument(
        "-o",
        "--output",
        default="-",
        help="SQL output path, or '-' for stdout (default: '-')",
    )
    parser.add_argument(
        "--replace-existing",
        action="store_true",
        help=(
            "remove presentation rows absent from the validated snapshot while "
            "preserving server-authored fields on surviving rows; cannot be "
            "combined with --exact-enable-selection"
        ),
    )
    parser.add_argument(
        "--validate-only",
        action="store_true",
        help="validate all files and relationships without emitting SQL",
    )
    parser.add_argument(
        "--strict-references",
        action="store_true",
        help=(
            "treat dangling cross-file references as errors instead of preserving "
            "them with warnings"
        ),
    )
    parser.add_argument(
        "--enable-through-expansion",
        metavar="EXPANSION",
        help=(
            "enable achievements categorized under every expansion from launch "
            "through EXPANSION (for example, 'don'); also enables validated "
            "General level, level-locked progression, class skill-cap, key, "
            "and era-appropriate epic milestones and emits all structurally "
            "validated criteria within the selected definitions"
        ),
    )
    parser.add_argument(
        "--max-level",
        type=int,
        help=(
            "override the selected expansion's level cap, or enable only "
            "validated General / Level, level-locked Progression, and "
            "class Skills milestones through this level"
        ),
    )
    parser.add_argument(
        "--max-tradeskill-skill",
        type=int,
        help=(
            "explicitly opt in to enabling only structurally validated direct "
            "Tradeskill milestones at or below this skill value and emit their "
            "SkillValue criteria"
        ),
    )
    parser.add_argument(
        "--max-aa-spent",
        type=int,
        help=(
            "explicitly opt in to enabling only structurally validated "
            "General / Advancement milestones at or below this spent-AA "
            "value and emit their AlternateAdvancement criteria"
        ),
    )
    parser.add_argument(
        "--exact-enable-selection",
        action="store_true",
        help=(
            "disable imported resource definitions outside the requested "
            "progression selection before enabling the selected IDs; custom "
            "achievement IDs absent from the resource snapshot are untouched; "
            "cannot be combined with --replace-existing"
        ),
    )
    parser.add_argument(
        "--enable-slayer",
        action="store_true",
        help=(
            "explicitly opt in to enabling only Slayer definitions whose "
            "complete client race-group shape maps exactly to RoF2 race IDs; "
            "emits incremental NpcRaceKill criteria and safely resolvable "
            "quoted-name meta dependencies"
        ),
    )
    parser.add_argument(
        "--preserve-enable-state",
        action="store_true",
        help=(
            "generate criteria from the requested progression selection while "
            "leaving every existing achievement enabled flag unchanged; "
            "selected definitions that do not exist yet are inserted enabled; "
            "cannot be combined with --exact-enable-selection"
        ),
    )
    return parser


def _selection_policy_warnings(
    *,
    enable_slayer: bool,
    through_expansion: Optional[str],
    preserve_enable_state: bool,
) -> Tuple[str, ...]:
    warnings: List[str] = []
    if enable_slayer and through_expansion is not None:
        warnings.append(
            "--enable-slayer selects the complete validated Slayer hierarchy; "
            f"--enable-through-expansion {through_expansion} does not "
            "constrain Slayer definitions"
        )
    if enable_slayer and preserve_enable_state:
        warnings.append(
            "--preserve-enable-state does not enable existing Slayer "
            "definitions; omit it when activating Slayer"
        )
    return tuple(warnings)


def main(argv: Optional[Sequence[str]] = None) -> int:
    arguments = _build_argument_parser().parse_args(argv)
    if arguments.exact_enable_selection and arguments.replace_existing:
        raise ResourceError(
            "--exact-enable-selection cannot be combined with "
            "--replace-existing because replacement removes custom "
            "definitions absent from the resource snapshot"
        )
    if arguments.exact_enable_selection and arguments.preserve_enable_state:
        raise ResourceError(
            "--exact-enable-selection cannot be combined with "
            "--preserve-enable-state"
        )
    for warning in _selection_policy_warnings(
        enable_slayer=arguments.enable_slayer,
        through_expansion=arguments.enable_through_expansion,
        preserve_enable_state=arguments.preserve_enable_state,
    ):
        print(f"warning: {warning}", file=sys.stderr)

    resources = load_resources(
        arguments.resource_directory.expanduser(),
        strict_references=arguments.strict_references,
    )
    selection = build_enable_selection(
        resources,
        through_expansion=arguments.enable_through_expansion,
        max_level=arguments.max_level,
        max_tradeskill_skill=arguments.max_tradeskill_skill,
        max_aa_spent=arguments.max_aa_spent,
        enable_slayer=arguments.enable_slayer,
    )
    selection_requested = (
        arguments.enable_through_expansion is not None
        or arguments.max_level is not None
        or arguments.max_tradeskill_skill is not None
        or arguments.max_aa_spent is not None
        or arguments.enable_slayer
    )
    if arguments.exact_enable_selection and not selection_requested:
        raise ResourceError(
            "--exact-enable-selection requires "
            "--enable-through-expansion, --max-level, or "
            "--max-tradeskill-skill, --max-aa-spent, or --enable-slayer"
        )
    if arguments.preserve_enable_state and not selection_requested:
        raise ResourceError(
            "--preserve-enable-state requires "
            "--enable-through-expansion, --max-level, "
            "--max-tradeskill-skill, --max-aa-spent, or --enable-slayer"
        )

    summary = (
        f"validated {len(resources.categories)} categories, "
        f"{len(resources.achievements)} achievements, "
        f"{len(resources.category_associations)} category associations, "
        f"{len(resources.components)} components, and "
        f"{len(resources.component_counts)} component counts"
    )
    if selection.achievement_ids:
        criteria_action = (
            "validated" if arguments.validate_only else "generated"
        )
        selection_purpose = (
            "for criteria generation"
            if arguments.preserve_enable_state
            else "for enablement"
        )
        summary += (
            f"; selected {len(selection.achievement_ids)} definitions "
            f"{selection_purpose} "
            f"({len(selection.expansion_achievement_ids)} expansion, "
            f"{len(selection.level_achievement_ids)} level, "
            f"{len(selection.progression_level_achievement_ids)} "
            "progression-level, "
            f"{len(selection.skill_cap_achievement_ids)} skill-cap, "
            f"{len(selection.item_achievement_ids)} additional item, "
            f"{len(selection.tradeskill_achievement_ids)} tradeskill, "
            f"{len(selection.aa_spent_achievement_ids)} AA-spent, "
            f"{len(selection.slayer_achievement_ids)} Slayer); "
            f"{criteria_action} {len(selection.generated_criteria)} fixed "
            f"criteria plus {len(selection.item_name_criteria)} item-name "
            "mappings "
            f"({len(selection.level_criteria)} level, "
            f"{selection.criteria_report.progression_level_generated} "
            "progression-level, "
            f"{selection.criteria_report.skill_cap_generated} skill-cap, "
            f"{selection.criteria_report.traveler_generated} traveler, "
            f"{selection.criteria_report.dependency_generated} dependency, "
            f"{selection.criteria_report.dependency_superseded} superseded "
            "dependency, "
            f"{selection.criteria_report.tradeskill_generated} tradeskill, "
            f"{selection.criteria_report.aa_spent_generated} AA-spent, "
            f"{selection.criteria_report.npc_name_generated} named-kill, "
            f"{selection.criteria_report.slayer_generated} Slayer)"
        )
        report = selection.criteria_report
        summary += (
            f"; skipped {report.progression_level_rejected} "
            "progression-level shapes, "
            f"{report.skill_cap_rejected} skill-cap shapes, "
            f"{report.item_definition_rejected} item definitions, "
            f"{report.traveler_rejected} traveler shapes, "
            f"{report.dependency_ambiguous} ambiguous dependencies, "
            f"{report.dependency_self} self dependencies, "
            f"{report.dependency_rejected} invalid dependencies, "
            f"{report.tradeskill_rejected} tradeskill shapes, "
            f"{report.aa_spent_rejected} AA-spent shapes, "
            f"{report.npc_name_unresolved_definitions} unresolved named-kill "
            f"definitions, {report.npc_name_rejected_components} invalid "
            f"named-kill components, and "
            f"{report.npc_name_collision_components} named-kill hash "
            f"collisions, and {report.slayer_rejected} Slayer definitions"
        )
        if arguments.exact_enable_selection:
            summary += "; exact imported-snapshot enablement requested"
        if arguments.preserve_enable_state:
            summary += "; existing achievement enable state preserved"

    if not arguments.validate_only:
        _write_output(
            arguments.output,
            generate_sql(
                resources,
                replace_existing=arguments.replace_existing,
                enable_selection=selection,
                exact_enable_selection=arguments.exact_enable_selection,
                preserve_enable_state=arguments.preserve_enable_state,
            ),
        )
    for warning in resources.warnings:
        print(f"warning: {warning}", file=sys.stderr)
    for rejection in selection.slayer_rejections:
        print(f"warning: {rejection}", file=sys.stderr)
    print(summary, file=sys.stderr)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ResourceError as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(2)
