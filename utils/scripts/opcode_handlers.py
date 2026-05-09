#! /usr/bin/env python
#

"""
'Opcode-Handlers' for EQEmulator

This script generates cross-references to show associated (handled) opcodes
between the server and client. It will generate files for each client and
server found, and provide some basic information..such as opcode names and
values, server handler and whether opcodes are translated on tx/rx, etc...

"""


import sys
import os
import fnmatch

from time import time, ctime


VERBOSE = False  # messaging: {False - minimal, True - robust}

base_path = os.getcwd()[:-14]  # '/utils/scripts'
base_path = base_path.replace('\\', '/')

client_list = ['Titanium', 'SoF', 'SoD', 'UF', 'RoF', 'RoF2']
server_list = ['Login', 'World', 'Zone', 'UCS', 'Query', 'EQLaunch', 'HeadlessClient', 'Common']  # 'Common' is not a server..but, may contain shared functions
server_dirs = {'Login': 'loginserver', 'World': 'world', 'Zone': 'zone', 'UCS': 'ucs', 'Query': 'queryserv', 'EQLaunch': 'eqlaunch', 'HeadlessClient': 'hc', 'Common': 'common'}
file_exts = ['cpp', 'h']

client_opcodes = {}  # x[key='Client'][key='OP_CodeName'](value='0x####')
server_opcodes = {}  # x[key='OP_CodeName'](value=<integer>) - opcodes apply to all servers
servertalk_opcodes = {}  # x[key='OP_CodeName'](value=<integer>) - opcodes apply to all servers

client_encodes = {}  # x[key='Client'](value='OP_CodeName')
client_decodes = {}  # x[key='Client'](value='OP_CodeName')

server_handlers = {}  # x[key='Server'][key='OP_CodeName'](value='[%X] Near Handler::ReferenceFunction')
servertalk_handlers = {}  # x[key='Server'][key='OP_CodeName'](value='[%X] Near Handler::ReferenceFunction')

out_files = {}  # x[key='Object'](value=<file_stream>)

#statistics = {}
#report_entries = {}


def main():
    """ Call TASK METHODS independently and track success """

    fault = False
    faults = []

    print('Initializing...')

    if not fault:
        fault = not createoutputdirectory()

        if fault:
            faults.append('createoutputdirectory()')

    if not fault:
        fault = not opendebugfile()

        if fault:
            faults.append('opendebugfile()')

    if not fault:
        fault = not openundefinedfile()

        if fault:
            faults.append('openundefinedfile()')

    if not fault:
        print('Loading source data...')

        if not fault:
            fault = not loadclientopcodes()

            if fault:
                faults.append('loadclientopcodes()')

        if not fault:
            fault = not loadserveropcodes()

            if fault:
                faults.append('loadserveropcodes()')

        if not fault:
            fault = not loadservertalkopcodes()

            if fault:
                faults.append('loadservertalkopcodes()')

        if not fault:
            fault = not loadclienttranslators()

            if fault:
                faults.append('loadclienttranslators()')

        if not fault:
            fault = not discoverserverhandlers()

            if fault:
                faults.append('discoverserverhandlers()')

        if not fault:
            fault = not discoverservertalkhandlers()

            if fault:
                faults.append('discoverservertalkhandlers()')

        if not fault:
            fault = not clearemptyserverentries()

            if fault:
                faults.append('clearemptyserverentries()')

    if not fault:
        print('Creating output streams...')

        if not fault:
            fault = not openoutputfiles()

            if fault:
                faults.append('openoutputfiles()')

    if not fault:
        print('Parsing opcode data...')

        if not fault:
            fault = not parseclientopcodedata()

            if fault:
                faults.append('parseclientopcodedata()')

        if not fault:
            fault = not parseserveropcodedata()

            if fault:
                faults.append('parseserveropcodedata()')

        if not fault:
            fault = not parseservertalkopcodedata()

            if fault:
                faults.append('parseservertalkopcodedata()')

    if not fault:
        print('Destroying output streams...')

    if not closeoutputfiles():
        fault = True
        faults.append('closeoutputfiles()')

    if not closedebugfile():
        fault = True
        faults.append('closedebugfile()')

    if fault and len(faults) > 0:
        message = 'Script failed due to errors in:\n'

        for entry in faults:
            message += '  {0}\n'.format(entry)

        print(message)

    return


def createoutputdirectory():
    """ Check for OUTPUT DIRECTORY - create if does not exist """

    try:
        output_path = '{0}/utils/scripts/opcode_handlers_output'.format(base_path)

        vprint(output_path)

        if not os.path.exists(output_path):
            os.mkdir(output_path)

        return True
    except:
        print('(Exception Error: {0}) createoutputdirectory()'.format(sys.exc_info()[0]))

        return False


def opendebugfile():
    """ DEBUG FILE should always open """

    try:
        file_name = '{0}/utils/scripts/opcode_handlers_output/DEBUG.txt'.format(base_path)

        vprint(file_name)

        out_files['DEBUG'] = open(file_name, 'w')

        dprint(
            '>> \'Opcode-Handler\' DEBUG dump file\n'
            '>> file generated @ {0}\n\n'.format(ctime(time()))
        )

        if VERBOSE:
            dprint(
                '->open: \'{0}\' in \'w\' mode\n'
                'leaving \'opendebugfile()\'\n\n'.format(file_name)
            )

        return True
    except:
        print('(Exception Error: {0}) opendebugfile()'.format(sys.exc_info()[0]))

        if 'DEBUG' in out_files:
            vprint('Closing DEBUG output file...')

            out_files['DEBUG'].close()

            del out_files['DEBUG']

        return False


def openundefinedfile():
    """ UNDEFINED FILE should always open """

    if VERBOSE:
        dprint('entering \'openundefinedfile()\'\n')

    try:
        file_name = '{0}/utils/scripts/opcode_handlers_output/UNDEFINED.txt'.format(base_path)

        vprint(file_name)

        out_files['UNDEFINED'] = open(file_name, 'w')

        uprint(
            '>> \'Opcode-Handler\' UNDEFINED dump file\n'
            '>> file generated @ {0}\n\n'.format(ctime(time()))
        )

        if VERBOSE:
            dprint(
                '->open: \'{0}\' in \'w\' mode\n'
                'leaving \'openundefinedfile()\'\n\n'.format(file_name)
            )

        return True
    except:
        print('(Exception Error: {0}) openundefinedfile()'.format(sys.exc_info()[0]))

        if 'UNDEFINED' in out_files:
            vprint('Closing UNDEFINED output file...')

            out_files['UNDEFINED'].close()

            del out_files['UNDEFINED']

        return False


def loadclientopcodes():
    """ Load CLIENT OPCODES into memory """

    if VERBOSE:
        dprint('entering \'loadclientopcodes()\'\n')

    bad_clients = []
    
    for client in client_list:
        try:
            short_name = '/patch_{0}.conf'.format(client)

            file_name = '{0}/utils/patches{1}'.format(
                base_path,
                short_name
            )
            
            vprint(file_name)

            with open(file_name, 'r') as data_file:
                if VERBOSE:
                    dprint('->open: \'{0}\' in \'r\' mode\n'.format(file_name))

                client_opcodes[client] = {}
                line_no = 0

                for data_line in data_file:
                    line_no += 1
                    key_begin = data_line.find('OP_')
                    key_end = data_line.find('=', key_begin)

                    if not key_begin == 0 or key_end < 0:
                        continue

                    val_begin = data_line.find('0x', key_end)
                    val_end = val_begin + 6

                    if val_begin < 0:
                        continue

                    value = int(data_line[(val_begin + 2):val_end].lower(), 16)

                    if value == 0:
                        if VERBOSE:
                            uprint('\nUNDEFINED OPCODE FOUND: ../utils/patches{0}({1}:{2}) [{3}][{4}] = {5}\n'.format(
                                short_name,
                                line_no,
                                key_begin,
                                client,
                                data_line[key_begin:key_end],
                                '0x{0}'.format(hex(value)[2:].zfill(4))
                            ))

                        continue

                    client_opcodes[client][data_line[key_begin:key_end]] = '0x{0}'.format(hex(value)[2:].zfill(4))

                    if VERBOSE:
                        dprint('../utils/patches{0}({1}:{2}) [{3}][{4}] = {5}\n'.format(
                            short_name,
                            line_no,
                            key_begin,
                            client,
                            data_line[key_begin:key_end],
                            client_opcodes[client][data_line[key_begin:key_end]]
                        ))

            data_file.close()

            if VERBOSE:
                dprint('->close: \'{0}\'\n'.format(file_name))

            if not len(client_opcodes[client]) > 0:
                bad_clients.append(client)
        except:
            print('(Exception Error: {0}) loadclientopcodes() [{1}]'.format(
                sys.exc_info()[0],
                client
            ))

            dprint('<-except: \'{0} [{1}]\'\n'.format(
                sys.exc_info()[0],
                client
            ))

            bad_clients.append(client)

    for bad_client in bad_clients:
        vprint('Deleting \'{0}\' client from search criteria...'.format(bad_client))
            
        client_list.remove(bad_client)

        dprint('->delete: \'{0}\' client\n'.format(bad_client))

        if bad_client in client_opcodes:
            vprint('Deleting stale entries for \'{0}\' client...'.format(bad_client))

            del client_opcodes[bad_client]

            dprint('->delete: \'{0}\' client opcode entries\n'.format(bad_client))

    if not len(client_list) > 0:
        print('Could not locate valid clients...')

        dprint('leaving \'loadclientopcodes(): NO VALID CLIENTS EXIST\'\n\n')

        return False

    if not len(client_opcodes) > 0:
        print('Could not locate client opcode lists...')

        dprint('leaving \'loadclientopcodes(): CLIENT OPCODES NOT FOUND\'\n\n')

        return False

    if VERBOSE:
        dprint('leaving \'loadclientopcodes()\'\n\n')

    return True


def loadserveropcodes():
    """ Load SERVER OPCODES into memory """

    if VERBOSE:
        dprint('entering \'loadserveropcodes()\'\n')

    try:
        server_opcodes['OP_Unknown'] = 0
        value = 1

        if VERBOSE:
            dprint('(manual) \'Servers\' [OP_Unknown] = {0}\n'.format(server_opcodes['OP_Unknown']))

        file_name = '{0}/common/emu_oplist.h'.format(base_path)

        vprint(file_name)

        with open(file_name, 'r') as data_file:
            if VERBOSE:
                dprint('->open: \'{0}\' in \'r\' mode\n'.format(file_name))

            line_no = 0

            for data_line in data_file:
                line_no += 1
                val_begin = data_line.find('OP_', 2)
                val_end = data_line.find(')', val_begin)

                if val_begin < 0 or val_end < 0:
                    continue

                if data_line[:1] == 'N':
                    server_opcodes[data_line[val_begin:val_end]] = value
                    value += 1

                    if VERBOSE:
                        dprint('../common/emu_oplist.h({0}:{1}) \'Servers\' [{2}] = {3}\n'.format(
                            line_no,
                            val_begin,
                            data_line[val_begin:val_end],
                            server_opcodes[data_line[val_begin:val_end]]
                        ))

        data_file.close()

        if VERBOSE:
            dprint('->close: \'{0}\'\n'.format(file_name))

        file_name = '{0}/common/mail_oplist.h'.format(base_path)

        vprint(file_name)

        with open(file_name, 'r') as data_file:
            if VERBOSE:
                dprint('->open: \'{0}\' in \'r\' mode\n'.format(file_name))

            line_no = 0

            for data_line in data_file:
                line_no += 1
                val_begin = data_line.find('OP_', 2)
                val_end = data_line.find(')', val_begin)

                if val_begin < 0 or val_end < 0:
                    continue

                if data_line[:1] == 'N':
                    server_opcodes[data_line[val_begin:val_end]] = value
                    value += 1

                    if VERBOSE:
                        dprint('../common/mail_oplist.h({0}:{1}) \'Servers\' [{2}] = {3}\n'.format(
                            line_no,
                            val_begin,
                            data_line[val_begin:val_end],
                            server_opcodes[data_line[val_begin:val_end]]
                        ))

        data_file.close()

        if VERBOSE:
            dprint('->close: \'{0}\'\n'.format(file_name))
    except:
        print('(Exception Error: {0}) loadserveropcodes()'.format(sys.exc_info()[0]))

        dprint('leaving \'loadserveropcodes(): EXCEPTION ERROR\'\n\n')

        return False

    if not len(server_opcodes) > 0:
        print('Could not locate server opcode list...')

        dprint('leaving \'loadserveropcodes(): SERVER OPCODES NOT FOUND\'\n\n')

        return False

    if VERBOSE:
        dprint('leaving \'loadserveropcodes()\'\n\n')

    return True


def loadservertalkopcodes():
    """ Load SERVERTALK OPCODES into memory """

    if VERBOSE:
        dprint('entering \'loadservertalkopcodes()\'\n')

    try:
        file_name = '{0}/common/servertalk.h'.format(base_path)

        vprint(file_name)

        with open(file_name, 'r') as data_file:
            if VERBOSE:
                dprint('->open: \'{0}\' in \'r\' mode\n'.format(file_name))

            line_no = 0

            for data_line in data_file:
                line_no += 1

                if not data_line[:7] == '#define':
                    continue

                key_begin = data_line.find('ServerOP_', 8)
                key_end = data_line.find('0x', key_begin)

                if key_begin < 0 or key_end < 0:
                    continue

                key_value = data_line[key_begin:key_end]
                key_value = key_value.rstrip()

                val_begin = key_end
                val_end = val_begin + 6

                servertalk_opcodes[key_value] = data_line[val_begin:val_end]
                
                if VERBOSE:
                    dprint('../common/servertalk.h({0}:{1}) \'Servers\' [{2}] = {3}\n'.format(
                        line_no,
                        key_begin,
                        key_value,
                        servertalk_opcodes[key_value]
                    ))

        data_file.close()

        if VERBOSE:
            dprint('->close: \'{0}\'\n'.format(file_name))

        
    except:
        print('(Exception Error: {0}) loadservertalkopcodes()'.format(sys.exc_info()[0]))

        dprint('leaving \'loadservertalkopcodes(): EXCEPTION ERROR\'\n\n')

        return False

    if not len(servertalk_opcodes) > 0:
        print('Could not locate servertalk opcode list...')

        dprint('leaving \'loadservertalkopcodes(): SERVERTALK OPCODES NOT FOUND\'\n\n')

        return False

    if VERBOSE:
        dprint('leaving \'loadservertalkopcodes()\'\n\n')

    return True


def loadclienttranslators():
    """
    Load CLIENT ENCODES and CLIENT DECODES OPCODES into memory

    The CLIENT LIST should be clean of any invalid entries by the time this
    function is called. Client translator load failures are only commented
    upon in output streams and do not trigger a removal of the client.

    """

    if VERBOSE:
        dprint('entering \'loadclienttranslators()\'\n')

    bad_clients = []

    for client in client_list:
        try:
            short_name = '/{0}_ops.h'.format(client).lower()

            file_name = '{0}/common/patches{1}'.format(
                base_path,
                short_name
            )
            
            vprint(file_name)

            with open(file_name, 'r') as data_file:
                if VERBOSE:
                    dprint('->open: \'{0}\' in \'r\' mode\n'.format(file_name))

                client_encodes[client] = []
                client_decodes[client] = []
                line_no = 0

                for data_line in data_file:
                    line_no += 1
                    val_begin = data_line.find('OP_', 2)
                    val_end = data_line.find(')', val_begin)

                    if val_begin < 0 or val_end < 0:
                        continue
                    
                    if data_line[:1] == 'E':
                        client_encodes[client].append(data_line[val_begin:val_end])

                        if VERBOSE:
                            dprint('..{0}({1}:{2}) \'ENCODE\' [{3}] = {4}\n'.format(
                                short_name,
                                line_no,
                                val_begin,
                                client,
                                data_line[val_begin:val_end]
                            ))
                    elif data_line[:1] == 'D':
                        client_decodes[client].append(data_line[val_begin:val_end])
                        
                        if VERBOSE:
                            dprint('..{0}({1}:{2}) \'DECODE\' [{3}] = {4}\n'.format(
                                short_name,
                                line_no,
                                val_begin,
                                client,
                                data_line[val_begin:val_end]
                            ))

            data_file.close()

            if VERBOSE:
                dprint('->close: \'{0}\'\n'.format(file_name))
        except:
            print('(Exception Error: {0}) loadclienttranslators() [{1}]'.format(
                sys.exc_info()[0],
                client
            ))

            dprint('<-except: \'{0} [{1}]\'\n'.format(
                sys.exc_info()[0],
                client
            ))

            bad_clients.append(client)

    for bad_client in bad_clients:
        if bad_client in client_encodes or bad_client in client_decodes:
            vprint('Deleting stale entries for \'{0}\' client...'.format(bad_client))

            if bad_client in client_encodes:
                del client_encodes[bad_client]

                dprint('->delete: \'{0}\' client encode entries\n'.format(bad_client))

            if bad_client in client_decodes:
                del client_decodes[bad_client]

                dprint('->delete: \'{0}\' client decode entries\n'.format(bad_client))

    if not len(client_encodes) > 0 and not len(client_decodes) > 0:
        dprint('leaving \'loadclienttranslators(): NO CLIENT ENCODES OR DECODES FOUND\'\n\n')

        return False

    if VERBOSE:
        dprint('leaving \'loadclienttranslators()\'\n\n')

    return True


def discoverserverhandlers():
    """
    Load SERVER OPCODE HANDLERS using 'discovery' method

    Lists are instantiated for all SERVERS in SERVER LIST. The lists are then appended
    with location data based on the presence of the SERVER in the parent dictionary.

    """

    # TODO: handle multi-line remark statements in file
    # TODO: if/how to include perl, lua and non-'../<server>/<file>' location handlers...

    if VERBOSE:
        dprint('entering \'discoverserverhandlers()\'\n')

    bad_servers = []
    locations = {}

    for server in server_list:
        if not server in locations:
            locations[server] = []

    for server in locations:
        file_path = '{0}/{1}/'.format(base_path, server_dirs[server])

        file_list = os.listdir(file_path)

        for extension in file_exts:
            if VERBOSE:
                dprint('->file discovery: \'{0}*.{1}\'\n'.format(file_path, extension))

            for file_name in fnmatch.filter(file_list, '*.{0}'.format(extension)):
                if file_name in ['emu_oplist.h', 'mail_oplist.h', 'opcode_dispatch.h', 'opcode_map.cpp', 'op_codes.h']:
                    continue

                locations[server].append('/{0}/{1}'.format(server_dirs[server], file_name))
    
    for server in server_list:
        if not server in server_handlers:
            server_handlers[server] = {}

        for location in locations[server]:
            try:
                file_name = '{0}{1}'.format(
                    base_path,
                    location)

                vprint(file_name)

                with open(file_name, 'r') as data_file:
                    if VERBOSE:
                        dprint('->open: \'{0}\' in \'r\' mode\n'.format(file_name))

                    line_no = 0
                    hint = 'Near beginning of file'

                    for data_line in data_file:
                        line_no += 1
                        read_begin = 0

                        if data_line[:1].isalpha():
                            hint_end = data_line.find('(')

                            if not hint_end < 0:
                                hint_begin = hint_end - 1

                                while not hint_begin < 0:
                                    if data_line[(hint_begin - 1):hint_begin].isspace():
                                        if not data_line[hint_begin:(hint_begin + 1)].isalpha():
                                            hint_begin += 1

                                        hint = 'Near {0}'.format(data_line[hint_begin:hint_end])

                                        break

                                    hint_begin -= 1

                        op_begin = data_line.find('OP_', read_begin)

                        if op_begin < 0:
                            continue

                        # exclusions
                        if data_line[(op_begin - 1):op_begin].isalnum():
                            continue
                        elif data_line[:op_begin].find('//', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nREMARKED OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('Log(Logs::', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nLOGGING OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('std::cout', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nCONSOLE MESSAGE OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('MakeAnyLenString', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nSTRING FORMAT OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('printf', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nSTRING FORMAT OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('VERIFY_PACKET_LENGTH', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nPACKET LENGTH OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('ConnectingOpcodes', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nOPCODE HANDLER ASSIGNMENT REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('ConnectedOpcodes', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nOPCODE HANDLER ASSIGNMENT REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('command_add', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nCOMMAND HANDLER ASSIGNMENT OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('luabind::value', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nLUA API OPCODE ASSIGNMENT REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].find('Message', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nCLIENT MESSAGE OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].count('"') and 1:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nSTRING TEXT OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[(op_begin - 3):op_begin] == '!= ':
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nILL-DEFINED OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[(op_begin - 2):op_begin] == '!=':
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nILL-DEFINED OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[(op_begin - 3):op_begin] == '>= ':
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nILL-DEFINED OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[(op_begin - 2):op_begin] == '>=':
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nILL-DEFINED OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[(op_begin - 3):op_begin] == '<= ':
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nILL-DEFINED OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[(op_begin - 2):op_begin] == '<=':
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nILL-DEFINED OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].isspace():
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 3

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nILL-DEFINED OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        # inclusion [RX]
                        elif data_line[(op_begin - 7):op_begin] == 'Handle_':
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 15):op_begin] == 'Handle_Connect_':
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 5):op_begin] == 'case ':
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 3):op_begin] == '== ':
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 2):op_begin] == '==':
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        # inclusions [TX]
                        elif data_line[(op_begin - 1):op_begin] == '(' and data_line[:op_begin].find('EQProtocolPacket', 0) >= 0:
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 2):op_begin] == ' (' and data_line[:op_begin].find('EQProtocolPacket', 0) >= 0:
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 1):op_begin] == '(' and data_line[:op_begin].find('EQApplicationPacket', 0) >= 0:
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 2):op_begin] == ' (' and data_line[:op_begin].find('EQApplicationPacket', 0) >= 0:
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 12):op_begin] == '->SetOpcode(':
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 11):op_begin] == '.SetOpcode(':
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 2):op_begin] == '= ':
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 1):op_begin] == '=':
                            key_begin = op_begin
                            key_end = key_begin + 3
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        # fall-through
                        else:
                            key_begin = op_begin
                            key_end = key_begin + 3

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1

                            uprint(
                                '\nUNDEFINED OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                '->line: \'{4}\'\n'.format(
                                    location,
                                    line_no,
                                    key_begin,
                                    data_line[key_begin:key_end],
                                    data_line[:-1]
                                )
                            )

                            continue

                        if key_end < 0:
                            continue

                        if not data_line[key_begin:key_end] in server_opcodes:
                            dprint('\nILLEGAL OPCODE FOUND: ..{0}({1}:{2}) \'{3}\'\n'.format(
                                location,
                                line_no,
                                key_begin,
                                data_line[key_begin:key_end]
                            ))

                            continue

                        if not data_line[key_begin:key_end] in server_handlers[server]:
                            server_handlers[server][data_line[key_begin:key_end]] = []

                        server_handlers[server][data_line[key_begin:key_end]].append(
                            '..{0}({1}:{2}) \'{3}\''.format(
                                location,
                                line_no,
                                key_begin,
                                '{0} {1}'.format(
                                    direction,
                                    hint
                                )
                            )
                        )

                        if VERBOSE:
                            dprint('..{0}({1}:{2}) [{3}][{4}] = \'{5} {6}\'\n'.format(
                                location,
                                line_no,
                                key_begin,
                                server,
                                data_line[key_begin:key_end],
                                direction,
                                hint
                            ))

                data_file.close()

                if VERBOSE:
                    dprint('->close: \'{0}\'\n'.format(file_name))
            except:
                print('(Exception Error: {0}) discoverserverhandlers() [{1}]'.format(
                    sys.exc_info()[0],
                    server
                ))

                dprint('<-except: \'{0} [{1}]\'\n'.format(
                    sys.exc_info()[0],
                    server
                ))

                if not server in bad_servers:
                    bad_servers.append(server)

    for bad_server in bad_servers:
        if bad_server in server_handlers:
            vprint('Deleting stale entries for \'{0}\' server...'.format(bad_server))

            del server_handlers[bad_server]

            dprint('->delete: \'{0}\' server discovered handler entries\n'.format(bad_server))

    if VERBOSE:
        dprint('leaving \'discoverserverhandlers()\'\n\n')

    return True


def discoverservertalkhandlers():
    """
    Load SERVERTALK OPCODE HANDLERS using 'discovery' method

    Lists are instantiated for all SERVERS in SERVER LIST. The lists are then appended
    with location data based on the presence of the SERVER in the parent dictionary.

    """

    # TODO: handle multi-line remark statements in file
    # TODO: if/how to include perl, lua and non-'../<server>/<file>' location handlers...

    if VERBOSE:
        dprint('entering \'discoverservertalkhandlers()\'\n')

    bad_servers = []
    locations = {}

    for server in server_list:
        if not server in locations:
            locations[server] = []

    for server in locations:
        file_path = '{0}/{1}/'.format(base_path, server_dirs[server])

        file_list = os.listdir(file_path)

        for extension in file_exts:
            if VERBOSE:
                dprint('->file discovery: \'{0}*.{1}\'\n'.format(file_path, extension))

            for file_name in fnmatch.filter(file_list, '*.{0}'.format(extension)):
                if file_name in ['emu_oplist.h', 'mail_oplist.h', 'opcode_dispatch.h', 'opcode_map.cpp', 'op_codes.h', 'servertalk.h']:
                    continue

                locations[server].append('/{0}/{1}'.format(server_dirs[server], file_name))
    
    for server in server_list:
        if not server in servertalk_handlers:
            servertalk_handlers[server] = {}

        for location in locations[server]:
            try:
                file_name = '{0}{1}'.format(
                    base_path,
                    location)

                vprint(file_name)

                with open(file_name, 'r') as data_file:
                    if VERBOSE:
                        dprint('->open: \'{0}\' in \'r\' mode\n'.format(file_name))

                    line_no = 0
                    hint = 'Near beginning of file'

                    for data_line in data_file:
                        line_no += 1
                        read_begin = 0

                        if data_line[:1].isalpha():
                            hint_end = data_line.find('(')

                            if not hint_end < 0:
                                hint_begin = hint_end - 1

                                while not hint_begin < 0:
                                    if data_line[(hint_begin - 1):hint_begin].isspace():
                                        if not data_line[hint_begin:(hint_begin + 1)].isalpha():
                                            hint_begin += 1

                                        hint = 'Near {0}'.format(data_line[hint_begin:hint_end])

                                        break

                                    hint_begin -= 1

                        op_begin = data_line.find('ServerOP_', read_begin)

                        if op_begin < 0:
                            continue

                        # exclusions
                        if data_line[(op_begin - 1):op_begin].isalnum():
                            continue
                        elif data_line[op_begin:].find('_Struct', 0) >= 0 and not data_line[op_begin:data_line[op_begin:].find('_Struct', 0)].isspace():
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 9

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nIll-DEFINED SERVERTALK OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[read_begin:op_begin].find('//', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 9

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nREMARKED SERVERTALK OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[read_begin:op_begin].find('Log(Logs::', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 9

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nLOGGING SERVERTALK OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[read_begin:op_begin].find('std::cout', 0) >= 0:
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 9

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nCONSOLE MESSAGE SERVERTALK OPCODE REFERENCE FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        elif data_line[:op_begin].isspace():
                            if VERBOSE:
                                key_begin = op_begin
                                key_end = key_begin + 9

                                while data_line[key_end:(key_end + 1)].isalnum():
                                    key_end += 1

                                dprint(
                                    '\nILL-DEFINED SERVERTALK OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                    '->line: \'{4}\'\n'.format(
                                        location,
                                        line_no,
                                        key_begin,
                                        data_line[key_begin:key_end],
                                        data_line[:-1]
                                    )
                                )

                            continue
                        # inclusions [RX]
                        elif data_line[(op_begin - 1):op_begin] == '(' and data_line[read_begin:op_begin].find('OnMessage', 0) >= 0:
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 2):op_begin] == ' (' and data_line[read_begin:op_begin].find('OnMessage', 0) >= 0:
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 5):op_begin] == 'case ':
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 3):op_begin] == '== ':
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 2):op_begin] == '==':
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[RX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        # inclusions [TX]
                        elif data_line[(op_begin - 1):op_begin] == '(' and data_line[read_begin:op_begin].find('ServerPacket', 0) >= 0:
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 2):op_begin] == ' (' and data_line[read_begin:op_begin].find('ServerPacket', 0) >= 0:
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 7):op_begin] == '->Send(':
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 2):op_begin] == '= ':
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        elif data_line[(op_begin - 1):op_begin] == '=':
                            key_begin = op_begin
                            key_end = key_begin + 9
                            direction = '[TX]'

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1
                        # fall-through
                        else:
                            key_begin = op_begin
                            key_end = key_begin + 9

                            while data_line[key_end:(key_end + 1)].isalnum():
                                key_end += 1

                            uprint(
                                '\nUNDEFINED SERVERTALK OPCODE CONDITIONAL FOUND: ..{0}({1}:{2}) \'{3}\'\n'
                                '->line: \'{4}\'\n'.format(
                                    location,
                                    line_no,
                                    key_begin,
                                    data_line[key_begin:key_end],
                                    data_line[:-1]
                                )
                            )

                            continue

                        if key_end < 0:
                            continue

                        if not data_line[key_begin:key_end] in servertalk_opcodes:
                            dprint('\nILLEGAL SERVERTALK OPCODE FOUND: ..{0}({1}:{2}) \'{3}\'\n'.format(
                                location,
                                line_no,
                                key_begin,
                                data_line[key_begin:key_end]
                            ))

                            continue

                        if not data_line[key_begin:key_end] in servertalk_handlers[server]:
                            servertalk_handlers[server][data_line[key_begin:key_end]] = []

                        servertalk_handlers[server][data_line[key_begin:key_end]].append(
                            '..{0}({1}:{2}) \'{3}\''.format(
                                location,
                                line_no,
                                key_begin,
                                '{0} {1}'.format(
                                    direction,
                                    hint
                                )
                            )
                        )

                        if VERBOSE:
                            dprint('..{0}({1}:{2}) [{3}][{4}] = \'{5} {6}\'\n'.format(
                                location,
                                line_no,
                                key_begin,
                                server,
                                data_line[key_begin:key_end],
                                direction,
                                hint
                            ))

                data_file.close()

                if VERBOSE:
                    dprint('->close: \'{0}\'\n'.format(file_name))
            except:
                print('(Exception Error: {0}) discoverservertalkhandlers() [{1}]'.format(
                    sys.exc_info()[0],
                    server
                ))

                dprint('<-except: \'{0} [{1}]\'\n'.format(
                    sys.exc_info()[0],
                    server
                ))

                if not server in bad_servers:
                    bad_servers.append(server)

    for bad_server in bad_servers:
        if bad_server in servertalk_handlers:
            vprint('Deleting stale entries for \'{0}\' server...'.format(bad_server))

            del servertalk_handlers[bad_server]

            dprint('->delete: \'{0}\' server discovered servertalk handler entries\n'.format(bad_server))

    if VERBOSE:
        dprint('leaving \'discoverservertalkhandlers()\'\n\n')

    return True


def clearemptyserverentries():
    """
    Delete SERVER OPCODE HANDLERS with no references

    Server methods are treated a little differently than client ones. Because of multiple
    search functions, we do not want to invalidate a server until all data-load processes
    are performed.

    This function cleans up and removes unused server references after all relevant
    processing is complete.

    """

    if VERBOSE:
        dprint('entering \'clearemptyserverentries()\'\n')

    bad_servers = []

    for server in server_list:
        if not server in server_handlers or not len(server_handlers[server]) > 0:
            bad_servers.append(server)
        else:
            bad_opcodes = []

            for opcode in server_handlers[server]:
                if len(server_handlers[server][opcode]) == 0:
                    bad_opcodes.append(opcodes)

            for bad_opcode in bad_opcodes:
                del server_handlers[server][bad_opcode]

            if len(server_handlers[server]) == 0:
                bad_servers.append(server)

    for bad_server in bad_servers:
        vprint('Deleting \'{0}\' server from search criteria...'.format(bad_server))

        server_list.remove(bad_server)

        dprint('->delete: \'{0}\' server\n'.format(bad_server))

        if bad_server in server_handlers:
            vprint('Deleting stale entries for \'{0}\' server...'.format(bad_server))

            del server_handlers[bad_server]

            dprint('->delete: \'{0}\' server handler entries\n'.format(bad_server))

    if not len(server_list) > 0:
        print('Could not locate valid servers...')

        dprint('leaving \'clearemptyserverentries(): NO VALID SERVERS EXIST\'\n\n')

        return False

    if not len(server_handlers) > 0:
        print('Could not locate server handlers...')

        dprint('leaving \'clearemptyserverentries(): SERVER HANDLERS NOT FOUND\'\n\n')

        return False

    if VERBOSE:
        dprint('leaving \'clearemptyserverentries()\'\n\n')

    return True


def openoutputfiles():
    """ Open OUTPUT FILES in 'w' mode - create/overwrite mode """

    if VERBOSE:
        dprint('entering \'openoutputfiles()\'\n')

    try:
        file_name = '{0}/utils/scripts/opcode_handlers_output/REPORT.txt'.format(base_path)

        vprint(file_name)

        out_files['REPORT'] = open(file_name, 'w')

        if VERBOSE:
            dprint('->open: \'{0}\' in \'w\' mode\n'.format(file_name))

        rprint(
            '>> \'Opcode-Handler\' REPORT file\n'
            '>> file generated @ {0}\n\n'.format(ctime(time()))
        )

        for client in client_list:
            file_name = '{0}/utils/scripts/opcode_handlers_output/{1}_opcode_handlers.txt'.format(
                base_path,
                client
            )
            
            vprint(file_name)
            
            out_files[client] = open(file_name, 'w')

            if VERBOSE:
                dprint('->open: \'{0}\' in \'w\' mode\n'.format(file_name))

            cprint(
                client,
                '>> \'Opcode-Handler\' analysis for \'{0}\' client\n'
                '>> file generated @ {1}\n\n'.format(
                    client,
                    ctime(time())
                )
            )

        for server in server_list:
            file_name = '{0}/utils/scripts/opcode_handlers_output/{1}_opcode_handlers.txt'.format(
                base_path,
                server
            )

            vprint(file_name)

            out_files[server] = open(file_name, 'w')

            if VERBOSE:
                dprint('->open: \'{0}\' in \'w\' mode\n'.format(file_name))

            sprint(
                server,
                '>> \'Opcode-Handler\' analysis for \'{0}\' server\n'
                '>> file generated @ {1}\n\n'.format(
                    server,
                    ctime(time())
                )
            )

        file_name = '{0}/utils/scripts/opcode_handlers_output/ServerTalk_opcode_handlers.txt'.format(base_path)

        vprint(file_name)

        out_files['ServerTalk'] = open(file_name, 'w')

        if VERBOSE:
            dprint('->open: \'{0}\' in \'w\' mode\n'.format(file_name))

        stprint(
            '>> \'Opcode-Handler\' analysis for ServerTalk\n'
            '>> file generated @ {0}\n\n'.format(ctime(time()))
        )
        
        if VERBOSE:
            dprint('leaving \'openoutputfiles()\'\n\n')

        return True
    except:
        print('(Exception Error: {0}) openoutputfiles()'.format(sys.exc_info()[0]))

        if 'REPORT' in out_files:
            vprint('Closing REPORT output file...')

            out_files['REPORT'].close()

            del out_files['REPORT']

        for client in client_list:
            if client in out_files:
                vprint('Closing {0} client output file...'.format(client))

                out_files[client].close()

                del out_files[client]

        for server in server_list:
            if server in out_files:
                vprint('Closing {0} server output file...'.format(server))

                out_files[server].close()

                del out_files[server]

        if 'ServerTalk' in out_files:
            vprint('Closing ServerTalk output file...')

            out_files['ServerTalk'].close()

            del out_files['ServerTalk']
        
        if VERBOSE:
            dprint('leaving \'openoutputfiles(): EXCEPTION ERROR\'\n\n')

        return False


def parseclientopcodedata():
    """ Process CLIENT OPCODE cross-link references """

    if VERBOSE:
        dprint('entering \'parseclientopcodedata()\'\n')

    for client in client_list:
        server_max_len = 0

        for server in server_list:
            if len(server) > server_max_len:
                server_max_len = len(server)

        client_keys = client_opcodes[client].keys()
        client_keys.sort()

        for client_opcode in client_keys:
            handled = client_opcode in server_opcodes

            if handled is True:
                encoded = client_opcode in client_encodes[client]
                decoded = client_opcode in client_decodes[client]
            else:
                encoded = 'N/A'
                decoded = 'N/A'

            message = 'Opcode: {0} ({1}) | Handled: {2} | Encoded: {3} | Decoded: {4}\n'.format(
                client_opcode,
                client_opcodes[client][client_opcode],
                handled,
                encoded,
                decoded
            )

            for server in server_list:
                if client_opcode in server_handlers[server] and len(server_handlers[server][client_opcode]) > 0:
                    handler_list = server_handlers[server][client_opcode]
                    handler_list.sort()

                    for handler_entry in handler_list:
                        message += '  Server: {0} ({1}) | Handler: {2}\n'.format(
                            server.ljust(len(server) + (server_max_len - len(server)), ' '),
                            '{0}'.format(server_opcodes[client_opcode]).zfill(4),
                            handler_entry
                        )
                else:
                    message += '  Server: {0} (0000) | Handler: N/A\n'.format(
                        server.ljust(len(server) + (server_max_len - len(server)), ' ')
                    )

            message += '\n'

            cprint(client, message)

        if VERBOSE:
            dprint('->parse: \'{0}\' client\n'.format(client))
    
    if VERBOSE:
        dprint('leaving \'parseclientopcodedata()\'\n\n')

    return True


def parseserveropcodedata():
    """ Process SERVER OPCODE cross-link references """

    if VERBOSE:
        dprint('entering \'parseserveropcodedata()\'\n')

    for server in server_list:
        client_max_len = 0

        for client in client_list:
            if len(client) > client_max_len:
                client_max_len = len(client)

        handler_keys = server_handlers[server].keys()
        handler_keys.sort()

        for handler_opcode in handler_keys:
            handler_list = server_handlers[server][handler_opcode]
            handler_list.sort()

            message = ''

            for handler_entry in handler_list:
                message += 'Opcode: {0} ({1}) | Handler: {2}\n'.format(
                    handler_opcode,
                    '{0}'.format(server_opcodes[handler_opcode]).zfill(4),
                    handler_entry)

            for client in client_list:
                if handler_opcode in client_opcodes[client]:
                    val1 = client_opcodes[client][handler_opcode]
                    val2 = 'True'
                    val3 = '{0}'.format(handler_opcode in client_encodes[client])
                    val4 = '{0}'.format(handler_opcode in client_decodes[client])
                else:
                    val1 = '0x0000'
                    val2 = 'False'
                    val3 = 'N/A'
                    val4 = 'N/A'

                message += '  Client: {0} ({1}) | Handled: {2} | Encoded: {3} | Decoded: {4}\n'.format(
                    client.ljust(len(client) + (client_max_len - len(client)), ' '),
                    val1,
                    val2.ljust(len(val2) + (len('False') - len(val2)), ' '),
                    val3.ljust(len(val3) + (len('False') - len(val3)), ' '),
                    val4.ljust(len(val4) + (len('False') - len(val4)), ' ')
                )

            message += '\n'

            sprint(server, message)

        if VERBOSE:
            dprint('->parse: \'{0}\' server\n'.format(server))

    if VERBOSE:
        dprint('leaving \'parseserveropcodedata()\'\n\n')

    return True


def parseservertalkopcodedata():
    """ Process SERVERTALK OPCODE references """

    if VERBOSE:
        dprint('entering \'parseservertalkopcodedata()\'\n')

    local_handlers = {}  # swap key order for local processing

    for keya in servertalk_handlers:
        for keyb in servertalk_handlers[keya]:
            if not keyb in local_handlers:
                local_handlers[keyb] = {}
            if not keya in local_handlers[keyb]:
                local_handlers[keyb][keya] = []

            local_handlers[keyb][keya] = servertalk_handlers[keya][keyb]

    opcode_keys = local_handlers.keys()
    opcode_keys.sort()

    for opcode_key in opcode_keys:
        server_keys = local_handlers[opcode_key].keys()
        server_keys.sort()

        message = ''

        for server_key in local_handlers[opcode_key]:
            handler_list = local_handlers[opcode_key][server_key]
            handler_list.sort()

            for handler_entry in handler_list:
                message += 'Opcode: {0} ({1}) | Handler: [{2}] {3}\n'.format(
                    opcode_key,
                    '{0}'.format(servertalk_opcodes[opcode_key]).zfill(4),
                    server_key,
                    handler_entry)

        message += '\n'

        stprint(message)

    if VERBOSE:
        dprint('->parse: ServerTalk\n')
        dprint('leaving \'parseservertalkopcodedata()\'\n\n')

    return True


def closeoutputfiles():
    """ Close OUTPUT FILES - excluding DEBUG FILE """

    if VERBOSE:
        dprint('entering \'closeoutputfiles()\'\n')

    if 'REPORT' in out_files:
        file_name = out_files['REPORT'].name

        out_files['REPORT'].close()

        del out_files['REPORT']

        if VERBOSE:
            dprint('->close: \'{0}\'\n'.format(file_name))

    if 'UNDEFINED' in out_files:
        file_name = out_files['UNDEFINED'].name

        out_files['UNDEFINED'].close()

        del out_files['UNDEFINED']

        if VERBOSE:
            dprint('->close: \'{0}\'\n'.format(file_name))

    for client in client_list:
        if client in out_files:
            file_name = out_files[client].name

            out_files[client].close()

            del out_files[client]

            if VERBOSE:
                dprint('->close: \'{0}\'\n'.format(file_name))

    for server in server_list:
        if server in out_files:
            file_name = out_files[server].name

            out_files[server].close()

            del out_files[server]

            if VERBOSE:
                dprint('->close: \'{0}\'\n'.format(file_name))

    if 'ServerTalk' in out_files:
        file_name = out_files['ServerTalk'].name

        out_files['ServerTalk'].close()

        del out_files['ServerTalk']

        if VERBOSE:
            dprint('->close: \'{0}\'\n'.format(file_name))

    if VERBOSE:
        dprint('leaving \'closeoutputfiles()\'\n\n')

    return True


def closedebugfile():
    """ Close DEBUG FILE - last performed action to catch late messages """

    if VERBOSE:
        dprint('entering \'closedebugfile()\'\n')

    if 'DEBUG' in out_files:
        file_name = out_files['DEBUG'].name

        if VERBOSE:
            dprint('closing \'{0}\'\n'.format(file_name))

        out_files['DEBUG'].close()

        del out_files['DEBUG']

    return True


def cprint(client, message):
    """ CLIENT PRINT helper function """

    if client in out_files:
        out_files[client].write(message)


def dprint(message):
    """ DEBUG PRINT helper function """

    if 'DEBUG' in out_files:
        out_files['DEBUG'].write(message)


def rprint(message):
    """ REPORT PRINT helper function """

    if 'REPORT' in out_files:
        out_files['REPORT'].write(message)


def sprint(server, message):
    """ SERVER PRINT helper function """

    if server in out_files:
        out_files[server].write(message)


def stprint(message):
    """ SERVERTALK PRINT helper function """

    if 'ServerTalk' in out_files:
        out_files['ServerTalk'].write(message)


def uprint(message):
    """ UNDEFINED PRINT helper function """

    if 'UNDEFINED' in out_files:
        out_files['UNDEFINED'].write(message)


def vprint(message):
    """ VERBOSE PRINT helper function """

    if VERBOSE:
        print(message)


if __name__ == '__main__':
    main()
