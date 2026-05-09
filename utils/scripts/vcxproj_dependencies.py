#! /usr/bin/env python
#

"""
'VCXProj-Dependencies' for EQEmulator

This script locates external dependency paths and generates lists for each
project. In addition, it will cross-check these lists to determine if any
discrepancies exist for any dependencies globally and across all projects.

"""


import sys
import os
import fnmatch

try:
    import xml.etree.cElementTree as ElementTree
except ImportError:
    import xml.etree.ElementTree as ElementTree

from time import time, ctime


QUIET_REPORT = True

include_projects = []
exclude_projects = ['VCTargetsPath', 'CompilerIdC', 'CompilerIdCXX']  # these three should be left in by default

base_path = os.getcwd()[:-14]  # '/utils/scripts'
base_path = base_path.replace('\\', '/')

file_extensions = ['vcxproj']
project_paths = []
master_dependencies = []
# {[project]:{[build]:{[resource]:{[reference]:[paths]}}}}
project_dependencies = {}

out_files = {}

col1 = '{0}'.format(' ' * 0)
col2 = '{0}'.format(' ' * 2)
col3 = '{0}'.format(' ' * 4)
col4 = '{0}'.format(' ' * 6)
col5 = '{0}'.format(' ' * 8)


def main():
    """ main """
    
    if not create_output_directory():
        exit()
    
    if not open_output_files():
        exit()
    
    print 'Locating project paths...'
    locate_project_paths()
    print '..project count: {0}'.format(len(project_paths))
    print 'Parsing project files...'
    parse_project_files()
    print 'Building master dependencies...'
    build_master_dependencies()
    print '..dependency count: {0}'.format(len(master_dependencies))
    print 'Checking for version discrepancies...'
    check_for_version_discrepancies()
    close_output_files()
    print '\n__fin__'
    
    return


def create_output_directory():
    """ Check for output directory - create if does not exist """
    
    try:
        output_path = '{0}/utils/scripts/vcxproj_dependencies_output'.format(base_path)
        if not os.path.exists(output_path):
            os.mkdir(output_path)
        
        return True
    
    except IOError:
        print('(Exception Error: {0}) create_output_directory()'.format(sys.exc_info()[0]))
        
        return False


def open_output_files():
    """ Open all output files """
    
    try:
        file_name = '{0}/utils/scripts/vcxproj_dependencies_output/ProjectPaths.txt'.format(base_path)
        out_files['ProjectPaths'] = open(file_name, 'w')
        file_name = '{0}/utils/scripts/vcxproj_dependencies_output/MasterDependencies.txt'.format(base_path)
        out_files['MasterDependencies'] = open(file_name, 'w')
        file_name = '{0}/utils/scripts/vcxproj_dependencies_output/ProjectDependencies.txt'.format(base_path)
        out_files['ProjectDependencies'] = open(file_name, 'w')
        file_name = '{0}/utils/scripts/vcxproj_dependencies_output/ContextTree.txt'.format(base_path)
        out_files['ContextTree'] = open(file_name, 'w')
        file_name = '{0}/utils/scripts/vcxproj_dependencies_output/DiscrepancyReport.txt'.format(base_path)
        out_files['DiscrepancyReport'] = open(file_name, 'w')
        for file in out_files:
            out_files[file].write('>> \'VCXProj-Dependencies\' {0} file\n'.format(file))
            out_files[file].write('>> file generated @ {0}\n\n'.format(ctime(time())))
        
        return True
    
    except IOError:
        print('(Exception Error: {0}) open_output_files()'.format(sys.exc_info()[0]))
        close_output_files()
        
        return False


def locate_project_paths():
    """ Locate vcxproj files in the build folder """
    
    for root, dirs, files in os.walk('{0}/build'.format(base_path)):
        for name in files:
            project = name.split('.')[0]
            if not len(include_projects) == 0 and project not in include_projects:
                continue
            if not len(exclude_projects) == 0 and project in exclude_projects:
                continue
            for extension in file_extensions:
                if fnmatch.fnmatch(name, '*.{0}'.format(extension)):
                    project_paths.append(os.path.join(root, name).replace('\\', '/').lower())
    for path in project_paths:
        out_files['ProjectPaths'].write('{0};\n'.format(path))
    
    return


def fixup_path(project_path, dependency_path):
    """ Fix-up malformed dependency paths """
    
    trailing = dependency_path.replace('\\', '/')
    if '../' in trailing:
        if trailing[:3] == '../':  # windows
            leading = project_path[:project_path.rfind('/')]
            while trailing[:3] == '../':
                leading = leading[:leading.rfind('/')]
                trailing = trailing[3:]
            trailing = trailing.lower()
            trailing = '{0}/{1}'.format(leading, trailing)
        else:  # unix
            print '..processing unix-style path fix-up'
            while '../' in trailing:
                backout = trailing.find('../')
                backdir = trailing.rfind('/', 0, backout - 1)
                trailing = trailing.replace(trailing[backdir:backout + 2], '', 1)
            trailing = trailing.lower()
    else:
        trailing = trailing.lower()
    
    return trailing


def parse_project_files():
    """ Parse each vcxproj file's xml data """
    
    for key1 in project_paths:
        with open(key1, 'r') as vcxproj_file:
            project_dependencies[key1] = {}
            xml_tree = ElementTree.ElementTree(file=vcxproj_file)
            for element1 in xml_tree.getroot():
                if not element1.tag[-19:] == 'ItemDefinitionGroup':
                    continue
                # add '.split('|')[0]' to remove the '|Win##' attribute
                key2 = element1.attrib['Condition'].split('==')[1][1:-1]
                project_dependencies[key1][key2] = {}
                for element2 in element1.getiterator():
                    if element2.tag[-9:] == 'ClCompile':
                        key3 = element2.tag[-9:]
                        project_dependencies[key1][key2][key3] = {}
                        for element3 in element2.getiterator():
                            if element3.tag[-28:] == 'AdditionalIncludeDirectories':
                                key4 = element3.tag[-28:]
                                project_dependencies[key1][key2][key3][key4] = []
                                paths = element3.text.split(';')
                                for path in paths:
                                    project_dependencies[key1][key2][key3][key4].append(fixup_path(key1, path))
                    elif element2.tag[-15:] == 'ResourceCompile':
                        key3 = element2.tag[-15:]
                        project_dependencies[key1][key2][key3] = {}
                        for element3 in element2.getiterator():
                            if element3.tag[-28:] == 'AdditionalIncludeDirectories':
                                key4 = element3.tag[-28:]
                                project_dependencies[key1][key2][key3][key4] = []
                                paths = element3.text.split(';')
                                for path in paths:
                                    project_dependencies[key1][key2][key3][key4].append(fixup_path(key1, path))
                    elif element2.tag[-4:] == 'Midl':
                        key3 = element2.tag[-4:]
                        project_dependencies[key1][key2][key3] = {}
                        for element3 in element2.getiterator():
                            if element3.tag[-28:] == 'AdditionalIncludeDirectories':
                                key4 = element3.tag[-28:]
                                project_dependencies[key1][key2][key3][key4] = []
                                paths = element3.text.split(';')
                                for path in paths:
                                    project_dependencies[key1][key2][key3][key4].append(fixup_path(key1, path))
                    elif element2.tag[-4:] == 'Link':
                        key3 = element2.tag[-4:]
                        project_dependencies[key1][key2][key3] = {}
                        for element3 in element2.getiterator():
                            if element3.tag[-22:] == 'AdditionalDependencies':
                                key4 = element3.tag[-22:]
                                project_dependencies[key1][key2][key3][key4] = []
                                paths = element3.text.split(';')
                                for path in paths:
                                    project_dependencies[key1][key2][key3][key4].append(fixup_path(key1, path))
                            if element3.tag[-28:] == 'AdditionalLibraryDirectories':
                                key4 = element3.tag[-28:]
                                project_dependencies[key1][key2][key3][key4] = []
                                paths = element3.text.split(';')
                                for path in paths:
                                    project_dependencies[key1][key2][key3][key4].append(fixup_path(key1, path))
        vcxproj_file.close()
    
    return


def build_master_dependencies():
    """ Build master dependencies list """
    
    def write(message):
        """ internal 'ProjectDependencies' write method - performed here so processing takes place after fix-up """
        
        out_files['ProjectDependencies'].write('{0}\n'.format(message))
        
        return
    
    for key1 in project_dependencies:
        write('{0}<Project Path="{1}">'.format(col1, key1))
        for key2 in project_dependencies[key1]:
            write('{0}<Build Type="{1}">'.format(col2, key2))
            for key3 in project_dependencies[key1][key2]:
                write('{0}<Resource Type="{1}">'.format(col3, key3))
                for key4 in project_dependencies[key1][key2][key3]:
                    write('{0}<Reference Type="{1}">'.format(col4, key4))
                    for path in project_dependencies[key1][key2][key3][key4]:
                        write('{0}{1}'.format(col4, path))
                        if path not in master_dependencies:
                            master_dependencies.append(path)
                    write('{0}</Reference>'.format(col4))
                write('{0}</Resource>'.format(col3))
            write('{0}</Build>'.format(col2))
        write('{0}</Project>'.format(col1))
    master_dependencies.sort()
    for path in master_dependencies:
        out_files['MasterDependencies'].write('{0}\n'.format(path))
    
    return


def check_for_version_discrepancies():
    """ Check for dependency version discrepancies """
    
    def twrite(message):
        """ internal 'ContextTree' write method """
        
        out_files['ContextTree'].write('{0}\n'.format(message))
        
        return
    
    def rwrite(message):
        """ internal 'DiscrepancyReport' write method """
        
        out_files['DiscrepancyReport'].write('{0}\n'.format(message))
        
        return
    
    libraries = [
        'mysql',
        'zlib',
        'perl',
        'lua',
        'boost',
        'sodium',
        'openssl'
    ]
    references = [
        'include',
        'source',
        'library'
    ]
    priorities = {
        0: 'NOT FOUND',
        1: 'install',
        2: 'dependencies',
        3: 'libs',
        4: 'vcpkg',
        5: 'static',
        6: 'submodule'
    }
    # use all lowercase for path description
    # use forward slash ('/') for directory name separators
    # use '|' token for multiple hints ('my_file_path_1|my_file_path_2')
    # use '!!' token for explicit argument ('/perl/core!!' will find '../perl/core' but not '../perl/core/perl512.lib')
    # use '##' token for joined hints ('my_file_##_1')
    # use '&&', '^' and '@' tokens for multiple argument hints ('my_file_&&path_1^path_2^path_3@')
    # (i.e., 'my_file_path_1|my_file_##_2|my_##_##&&_3^_4!!@')
    # {[library]:{[reference]:[[priority]:hint]}}
    hints = {
        # Notes:
        'mysql': {
            'include': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/dependencies/mysql_##/include',  # 'dependencies'
                '',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'source': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '',  # 'dependencies'
                '',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'library': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                'dependencies/mysql_##/lib',  # 'dependencies'
                '',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ]
        },
        'zlib': {
            'include': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/zlib_x##/include',  # 'dependencies'
                # not sure if this should be '/libs/zlibng' or '/build/libs/zlibng' based on cmake behavior
                '/server/build/libs/zlibng!!',  # 'libs'
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/include',  # 'vcpkg'
                '/server/build/libs/zlibng!!',  # 'static'
                ''  # 'submodule'
            ],
            'source': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '',  # 'dependencies'
                '/server/libs/zlibng!!',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'library': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/zlib_x##/lib/zdll.lib',  # 'dependencies'
                '',  # 'libs'
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/&&lib/zlib.lib!!'
                '^debug/lib/zlibd.lib!!@',  # 'vcpkg'
                '/server/build/libs/zlibng/&&debug/zlibstaticd.lib!!^minsizerel/zlibstatic.lib!!'
                '^release/zlibstatic.lib!!^relwithdebinfo/zlibstatic.lib!!@',  # 'static'
                ''  # 'submodule'
            ]
        },
        'perl': {
            'include': [
                '',  # 'NOT FOUND'
                '/perl/lib/core!!',  # 'install'
                '',  # 'dependencies'
                '',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'source': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '',  # 'dependencies'
                '',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'library': [
                '',  # 'NOT FOUND'
                '/perl/lib/core/perl51##.lib',  # 'install'
                '',  # 'dependencies'
                '',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ]
        },
        'lua': {
            'include': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/luaj_x##/src',  # 'dependencies'
                '',  # 'libs'
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/include',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'source': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/luaj_x##/src',  # 'dependencies'
                '',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'library': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/luaj_x##/bin/lua51.lib',  # 'dependencies'
                '',  # 'libs'
                # debug lua package likely incorrect..should be 'lua51d.lib' - or whatever debug version is
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/&&lib/lua51.lib!!'
                '^debug/lib/lua51.lib!!@',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ]
        },
        'boost': {
            'include': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/boost',  # 'dependencies'
                '',  # 'libs'
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/include',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'source': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '',  # 'dependencies'
                '',  # 'libs'
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/include',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'library': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/boost',  # 'dependencies'
                '',  # 'libs'
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/lib!!',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ]
        },
        'sodium': {
            'include': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/libsodium/include',  # 'dependencies'
                '',  # 'libs'
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/include',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'source': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '',  # 'dependencies'
                '',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'library': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/libsodium/##/dynamic/libsodium.lib',  # 'dependencies'
                '',  # 'libs'
                # debug libsodium package likely incorrect..should be 'libsodiumd.lib' - or whatever debug version is
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/&&lib/libsodium.lib!!^'
                'debug/lib/libsodium.lib!!@',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ]
        },
        'openssl': {
            'include': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/openssl_x##/include',  # 'dependencies'
                '',  # 'libs'
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/include',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'source': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '',  # 'dependencies'
                '',  # 'libs'
                '',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ],
            'library': [
                '',  # 'NOT FOUND'
                '',  # 'install'
                '/server/dependencies/openssl_x##/lib/VC/&&libeay32MD.lib!!^libeay32MDd.lib!!^'
                     'ssleay32MD.lib!!^ssleay32MDd.lib!!@',  # 'dependencies'
                '',  # 'libs'
                # debug openssl package likely incorrect..should be
                # 'libeay32d.lib' and 'ssleay32d.lib' - or whatever debug versions are
                '/server/vcpkg/vcpkg-export-##/installed/x##-windows/&&lib/libeay32.lib!!^'
                     'lib/ssleay32.lib!!^debug/lib/libeay32.lib!!^debug/lib/ssleay32.lib!!@',  # 'vcpkg'
                '',  # 'static'
                ''  # 'submodule'
            ]
        }
    }
    # {[project]:{[build]:{[resource]:{[library]:{[reference]:priority}}}}}
    context_tree = {}
    # {[library]:priority}
    global_priorities = {}
    # {[build]:{[library]:priority}}
    build_priorities = {}
    # loop for discovering first occurence dependency sources (assumes same search precedence as compiler includes)
    for project in project_dependencies:
        if project not in context_tree.keys():
            context_tree[project] = {}
        for build in project_dependencies[project]:
            if build not in context_tree[project].keys():
                context_tree[project][build] = {}
            if build not in build_priorities.keys():
                build_priorities[build] = {}
            for resource in project_dependencies[project][build]:
                if resource not in context_tree[project][build].keys():
                    context_tree[project][build][resource] = {}
                for reference_project in project_dependencies[project][build][resource]:
                    for path in project_dependencies[project][build][resource][reference_project]:
                        for library in libraries:
                            if library not in context_tree[project][build][resource].keys():
                                context_tree[project][build][resource][library] = {}
                            if library not in build_priorities[build].keys():
                                build_priorities[build][library] = 0
                            if library not in global_priorities.keys():
                                global_priorities[library] = 0
                            for reference in references:
                                if reference not in context_tree[project][build][resource][library].keys():
                                    context_tree[project][build][resource][library][reference] = 0
                                elif not context_tree[project][build][resource][library][reference] == 0:
                                    continue
                                for priority in priorities:
                                    if hints[library][reference][priority] == '':
                                        continue
                                    hint_found = False
                                    for hint in hints[library][reference][priority].split('|'):
                                        if not find_hint_in_path(hint, path) == -1:
                                            context_tree[project][build][resource][library][reference] = priority
                                            if context_tree[project][build][resource][library][reference] >\
                                                    build_priorities[build][library]:
                                                build_priorities[build][library] =\
                                                    context_tree[project][build][resource][library][reference]
                                            if context_tree[project][build][resource][library][reference] >\
                                                    global_priorities[library]:
                                                global_priorities[library] =\
                                                    context_tree[project][build][resource][library][reference]
                                            hint_found = True
                                            break
                                    if hint_found is True:
                                        break
    # loop for hack to fix odd behavior caused by 'FindZLIB.cmake' - ref: '../server/build/libs/zlibng/zconf.h'
    # this does not change anything in the build files..only silences a false discrepancy due to mixing priority types
    if global_priorities['zlib'] == 5:
        for project in context_tree:
            for build in context_tree[project]:
                for resource in context_tree[project][build]:
                    if context_tree[project][build][resource]['zlib']['source'] == 3:
                        context_tree[project][build][resource]['zlib']['source'] = 5
                    if context_tree[project][build][resource]['zlib']['include'] == 3:
                        context_tree[project][build][resource]['zlib']['include'] = 5
    # loop for dumping 'global_priorities'
    twrite('{0}<Global>'.format(col1))
    for library in libraries:
        twrite('{0}<Library Name="{1}">{2}</Library>'.format(col2, library, global_priorities[library]))
    twrite('{0}</Global>'.format(col1))
    twrite('')
    # loop for dumping 'build_priorities'
    for build in build_priorities:
        twrite('{0}<Build Type="{1}">'.format(col1, build))
        for library in libraries:
            twrite('{0}<Library Name="{1}">{2}</Library>'.format(col2, library, build_priorities[build][library]))
        twrite('{0}</Build>'.format(col1))
    twrite('')
    # loop for dumping 'context_tree'
    for project in context_tree:
        twrite('{0}<Project Path="{1}">'.format(col1, project))
        for build in context_tree[project]:
            twrite('{0}<Built Type="{1}">'.format(col2, build))
            for resource in context_tree[project][build]:
                twrite('{0}<Resource Name="{1}">'.format(col3, resource))
                for library in context_tree[project][build][resource]:
                    twrite('{0}<Library Name="{1}">'.format(col4, library))
                    for reference in context_tree[project][build][resource][library]:
                        twrite(
                            '{0}<Reference Name="{1}">{2}</Reference>'.format(
                                col5,
                                reference,
                                context_tree[project][build][resource][library][reference]
                            )
                        )
                    twrite('{0}</Library>'.format(col4))
                twrite('{0}</Resource>'.format(col3))
            twrite('{0}</Build>'.format(col2))
        twrite('{0}</Project>'.format(col1))
    if QUIET_REPORT is False:
        for library in libraries:
            rwrite(
                '> Global Library \'{0}\' status: \'{1}\' ({2})'.format(
                    library,
                    priorities[global_priorities[library]],
                    global_priorities[library]
                )
            )
    # loop for identifying dependency discrepancies
    for project in context_tree:
        for build in context_tree[project]:
            for resource in context_tree[project][build]:
                for library in context_tree[project][build][resource]:
                    if global_priorities[library] == 0:
                        if QUIET_REPORT is False:
                            rwrite(
                                '> No Global Library \'{0}\' .. skipping Project:Build:Resource'
                                ' "{1}":"{2}":"{3}"'.format(
                                    library,
                                    project,
                                    build,
                                    resource
                                )
                            )
                        continue
                    if build_priorities[build][library] == 0:
                        if QUIET_REPORT is False:
                            rwrite(
                                '> No Build Library \'{0}\' .. skipping Project:Build:Resource'
                                ' "{1}":"{2}":"{3}"'.format(
                                    library,
                                    project,
                                    build,
                                    resource
                                )
                            )
                        continue
                    for reference in context_tree[project][build][resource][library]:
                        if context_tree[project][build][resource][library][reference] == 0:
                            continue
                        if not global_priorities[library] == context_tree[project][build][resource][library][reference]:
                            rwrite(
                                '> Global-Project Library \'{0}\' mis-match \'{1}!={2}\''
                                ' ({3}!={4}) Project:Build:Resource "{5}":"{6}":"{7}"'.format(
                                    library,
                                    priorities[global_priorities[library]],
                                    priorities[context_tree[project][build][resource][library][reference]],
                                    global_priorities[library],
                                    context_tree[project][build][resource][library][reference],
                                    project,
                                    build,
                                    resource
                                )
                            )
                        # 'builds' are allowed to have different dependencies..so, we'll start crossing at 'resource'
                        for cross_resource in context_tree[project][build]:
                            for cross_reference in context_tree[project][build][cross_resource][library]:
                                if cross_resource == resource and cross_reference == reference:
                                    continue
                                if context_tree[project][build][cross_resource][library][cross_reference] == 0:
                                    continue
                                if QUIET_REPORT is False and\
                                        not context_tree[project][build][cross_resource][library][cross_reference] ==\
                                        context_tree[project][build][resource][library][reference]:
                                    rwrite(
                                        '> Project Library \'{0}\' mis-match \'{1}:{2}:{3}!={4}:{5}:{6}\''
                                        ' ({7}!={8}) Project:Build "{9}":"{10}"'.format(
                                            library,
                                            resource,
                                            reference,
                                            priorities[context_tree[project][build][resource][library][reference]],
                                            cross_resource,
                                            cross_reference,
                                            priorities[context_tree[project][build][cross_resource][library]
                                                       [cross_reference]],
                                            context_tree[project][build][resource][library][reference],
                                            context_tree[project][build][cross_resource][library][cross_reference],
                                            project,
                                            build
                                        )
                                    )
    
    return


def find_hint_in_path(hint, path):
    """
    Helper function for parsing and checking for hints in paths
    
    Hints strings should be split ('|') and passed as a singular hint into this function
    
    """
    
    if hint == '' or path == '':
        return -1
    
    joined_index = hint.find('##')
    pretext_index = hint.find('&&')
    if joined_index == -1 and pretext_index == -1:
        if '^' in hint or '@' in hint:
            print '..malformed or improper handling of hint: \'{0}\' path: \'{1}\''.format(hint, path)
            
            return -1
        
        explicit_index = hint.find('!!')
        if explicit_index == -1:
            return path.find(hint)
        
        else:
            explicit_hint = hint[:explicit_index]
            found_index = path.find(explicit_hint)
            if (len(explicit_hint) + found_index) == len(path):
                return found_index
        
            else:
                return -1
    
    elif (not joined_index == -1 and pretext_index == -1) or\
            (not joined_index == -1 and not pretext_index == -1 and joined_index < pretext_index):
        start_index = 0
        for partial_hint in hint.split('##', 1):
            if partial_hint == '':
                continue
            found_index = find_hint_in_path(partial_hint, path[start_index:])
            if found_index == -1:
                return found_index
            
            start_index = found_index + len(partial_hint)
        
        return start_index
    
    elif (joined_index == -1 and not pretext_index == -1) or\
            (not joined_index == -1 and not pretext_index == -1 and joined_index > pretext_index):
        pretext_hints = hint.split('&&', 1)
        found_index = 0
        if not pretext_hints[0] == '':
            found_index = find_hint_in_path(pretext_hints[0], path)
            if found_index == -1:
                return found_index
        
        start_index = found_index + len(pretext_hints[0])
        partial_hints = pretext_hints[1].split('@', 1)
        for partial_hint in partial_hints:
            if partial_hint == '':
                continue
            for alt_hint in partial_hint.split('^'):
                if alt_hint == '':
                    continue
                found_index = find_hint_in_path(alt_hint, path[start_index:])
                if found_index == 0:
                    if not partial_hints[1] == '':
                        print '..unhandled hint method: \'{0}\''.format(partial_hints[1])
                    else:
                        return found_index
        
        return -1
        
    else:
        return -1


def close_output_files():
    """ Close all output files """
    
    while not len(out_files) == 0:
        key = out_files.keys()[0]
        out_files[key].close()
        del out_files[key]
    
    return


if __name__ == '__main__':
    main()
