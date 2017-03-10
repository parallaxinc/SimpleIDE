#!/usr/bin/env /usr/bin/python3
import platform

import argparse
import os
import shutil
import subprocess
import multiprocessing
import tarfile
import urllib.request

NAME = 'SimpleIDE'
SIMPLE_IDE_SOURCE_ROOT = os.path.dirname(os.path.realpath(__file__))
DEFAULT_BUILD_PATH = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'build')
DEFAULT_PROPGCC_PATH = os.path.join(DEFAULT_BUILD_PATH, 'parallax')

PROP_LOADER_PATH = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'PropLoader')
CTAGS_PATH = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'ctags-5.8')
SETUP_SCRIPT = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'release', 'linux', 'setup.sh')
SETUP_RUN_SCRIPT = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'release', 'linux', 'simpleide.sh')
SPIN_LIBRARIES = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'spin')
PROPSIDE_PATH = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'propside')
WX_LOADER = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'PropLoader')

MAKE_JOBS_ARG = '-j%d' % (multiprocessing.cpu_count() + 1)


class QmakeNotFoundException(Exception):
    def __init__(self):
        super().__init__('Qt must be installed. Please install Qt5.4 from here:' + os.linesep +
                         'download.qt.io./official_releases/qt/5.4/5.4.2')


class Qt5NotAvailableException(Exception):
    def __init__(self):
        super().__init__('The qmake program must be Qt5.4 or higher vintage.' + os.linesep +
                         'Please adjust PATH to include a Qt5 build if necessary.' + os.linesep +
                         'Please install Qt5.4 from here if you don\'t have it already:' + os.linesep +
                         'download.qt.io./official_releases/qt/5.4/5.4.2')


def run():
    args = parse_args()

    binary_root = args.build
    user_propgcc = args.propgcc
    no_clean = args.no_clean

    package_binary_path = create_package_path(binary_root)
    package_name = os.path.basename(package_binary_path)

    install_propgcc(binary_root, user_propgcc, package_binary_path)
    compile_proploader(package_binary_path)
    simple_ide_binary = compile_simple_ide(binary_root, package_binary_path)
    compile_ctags(binary_root, package_binary_path)
    install_shared_libs(package_binary_path, simple_ide_binary)
    install_static_files(package_binary_path)

    if not no_clean:
        print("Make tarball...")
        archive_name = '%s.%s.%s-linux.tar.bz2' % (package_name, platform.machine(), platform.uname().node)
        archive_path = os.path.join(binary_root, archive_name)
        if os.path.exists(archive_path):
            os.remove(archive_path)
        with tarfile.open(archive_path, 'w:bz2') as archive:
            archive.add(package_binary_path, arcname=package_name)

    print('All done.')


def install_propgcc(binary_root, user_propgcc, package_binary_path):
    if user_propgcc and os.path.exists(user_propgcc):
        propgcc_path = user_propgcc
    else:
        propgcc_url = 'http://david.zemon.name:8111/repository/download/PropGCC5_Gcc4linuxX64/.lastSuccessful/propellergcc-alpha_v1_9_0-gcc4-linux-x64.tar.gz?guest=1'
        propgcc_archive_name = os.path.join(binary_root, 'propgcc.tar.gz')

        print('Downloading PropGCC from %s%s\tto %s' % (propgcc_url, os.linesep, propgcc_archive_name))
        urllib.request.urlretrieve(propgcc_url, propgcc_archive_name)

        print('Extracting PropGCC archive...')
        with tarfile.open(propgcc_archive_name, "r:gz") as propgcc_archive:
            propgcc_archive.extractall(binary_root)

        propgcc_path = DEFAULT_PROPGCC_PATH

    propgcc_target_path = os.path.join(package_binary_path, 'parallax')
    shutil.copytree(propgcc_path, propgcc_target_path)
    os.environ['PATH'] = '%s/bin:%s' % (propgcc_target_path, os.environ['PATH'])


def compile_simple_ide(binary_root, package_binary_path):
    qmake_args = get_qmake_invocation()
    propside_binary_root = os.path.join(binary_root, 'propside')

    os.makedirs(propside_binary_root, exist_ok=True)
    invoke(qmake_args + [PROPSIDE_PATH], propside_binary_root)
    invoke(['make', 'clean'], propside_binary_root)
    invoke(['make', MAKE_JOBS_ARG], propside_binary_root)

    simple_ide_binary = os.path.join(propside_binary_root, 'SimpleIDE')
    install_binary(simple_ide_binary, package_binary_path)
    return simple_ide_binary


def compile_proploader(package_binary_path):
    environment_vars = {}
    environment_vars.update(os.environ)
    environment_vars['OS'] = 'linux'

    invoke(['make', 'clean'], cwd=PROP_LOADER_PATH, env=environment_vars)
    invoke(['make', MAKE_JOBS_ARG], cwd=PROP_LOADER_PATH, env=environment_vars)

    proploader_binary = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'proploader-linux-build', 'bin', 'proploader')
    install_binary(proploader_binary, package_binary_path)


def compile_ctags(binary_root, package_binary_path):
    ctags_binary_path = os.path.join(binary_root, 'ctags-bin')
    if os.path.exists(ctags_binary_path):
        shutil.rmtree(ctags_binary_path)
    os.makedirs(ctags_binary_path)

    invoke([os.path.join(CTAGS_PATH, 'configure')], cwd=ctags_binary_path, output=False)
    invoke(['make', MAKE_JOBS_ARG], cwd=ctags_binary_path)

    ctags_binary = os.path.join(ctags_binary_path, 'ctags')
    install_binary(ctags_binary, package_binary_path)


def create_package_path(binary_root):
    with open(os.path.join(PROPSIDE_PATH, 'propside.pro'), 'r') as propside_project_file:
        lines = propside_project_file.readlines()
    version_lines = [line.strip() for line in lines if 'VERSION=' in line]
    version_numbers = [line.split('VERSION=')[1] for line in version_lines]
    full_version_number = '.'.join(version_numbers)
    full_version = '%s-%s' % (NAME, full_version_number)
    print('Creating Version ' + full_version)
    package_binary_path = os.path.join(binary_root, full_version)

    shutil.rmtree(package_binary_path)
    os.makedirs(package_binary_path)
    
    return package_binary_path


def install_static_files(package_binary_path):
    release_template_path = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'release', 'template')
    for source_root, dirs, files in os.walk(release_template_path):
        relative_root = source_root[len(release_template_path):]
        if relative_root.startswith('/'):
            relative_root = relative_root[1:]

        destination_root = os.path.join(package_binary_path, relative_root)
        if not os.path.exists(destination_root):
            os.makedirs(destination_root, exist_ok=True)

        for f in files:
            source = os.path.join(source_root, f)
            shutil.copy2(source, destination_root)
    setup_script = shutil.copy2(SETUP_SCRIPT, package_binary_path)
    os.chmod(setup_script, 0o744)
    install_binary(SETUP_RUN_SCRIPT, package_binary_path)
    shutil.copytree(os.path.join(PROPSIDE_PATH, 'translations'), os.path.join(package_binary_path, 'translations'))
    shutil.copy2(os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'SimpleIDE-User-Guide.pdf'),
                 os.path.join(package_binary_path, 'parallax', 'bin'))
    shutil.copytree(os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'propside-demos'), os.path.join(package_binary_path, 'demos'))

    workspace_dir = os.path.join(SIMPLE_IDE_SOURCE_ROOT, 'Workspace')
    subprocess.check_call(['git', 'fetch'], cwd=workspace_dir)
    subprocess.check_call(['git', 'pull'], cwd=workspace_dir)
    shutil.copytree(workspace_dir, os.path.join(package_binary_path, 'parallax', 'Workspace'))


def install_shared_libs(package_binary_path, simpleide_binary):
    all_libs = subprocess.check_output(['ldd', simpleide_binary]).decode().split(os.linesep)
    qt_libs = [lib.strip().split()[2] for lib in all_libs if 'libQt' in lib or 'libaudio' in lib]
    for lib in qt_libs:
        install_binary(lib, package_binary_path)


def invoke(args, cwd, env=None, output=True):
    print('%s$ %s' % (cwd, ' '.join(args)))
    if output:
        subprocess.check_call(args, cwd=cwd, env=env)
    else:
        print('\tstdout hidden...')
        subprocess.check_output(args, cwd=cwd, env=env)


def install_binary(binary, package_root):
    package_binary_bin_path = os.path.join(package_root, 'bin')
    destination_path = os.path.join(package_binary_bin_path, os.path.basename(binary))
    if not os.path.exists(package_binary_bin_path):
        os.makedirs(package_binary_bin_path)
    shutil.copy2(binary, destination_path)
    os.chmod(destination_path, 0o755)
    return destination_path


def get_qmake_invocation():
    qmake_path = which('qmake')
    if not qmake_path:
        raise QmakeNotFoundException

    real_qmake_path = os.path.realpath(qmake_path)
    if 'qtchooser' == os.path.basename(real_qmake_path):
        raw_output = subprocess.check_output([real_qmake_path, '-list-versions'])
        qt_versions = raw_output.decode().split()
        if '5' in qt_versions or 'qt5' in qt_versions:
            return [qmake_path, '-qt=5']
        else:
            raise Qt5NotAvailableException()
    else:
        raw_output = subprocess.check_output([qmake_path, '-version'])
        string_output = raw_output.decode()
        if 'qt5' in string_output.lower():
            return [qmake_path]
        else:
            raise Qt5NotAvailableException()


def find(name, default_paths, search_path):
    """
    Find a file
    :param name: Name of the file to find 
    :type name str
    :param default_paths: Default paths where the file might be located
    :type default_paths list
    :param search_path: Root directory where a search should be performed if the file is not in the default location
    :type search_path str
    :return: Returns the path of the requested file
    :raises IOError If the file cannot be found
    """
    for default in default_paths:
        default = os.path.join(default, name)
        if os.path.exists(default):
            return os.path.realpath(default)
    for root, dirs, files in os.walk(search_path):
        if name in files:
            return os.path.realpath(os.path.join(root, name))


def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)


def which(program):
    file_path, filename = os.path.split(program)
    if file_path:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file

    return None


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-p', '--package', default=NAME + '.zip',
                        help='Name (and path) of the compressed package that will be generated')
    parser.add_argument('-g', '--propgcc', default=DEFAULT_PROPGCC_PATH,
                        help='PropGCC installation path on the local machine. If not provided, the latest PropGCC '
                             'build from TeamCity will be downloaded and extracted for inclusion in the SimpleIDE '
                             'package.')
    parser.add_argument('-b', '--build', default=DEFAULT_BUILD_PATH,
                        help='Directory where the build for SimpleIDE can take place')
    parser.add_argument('--no-clean', action='store_true', help='When given, the tarball will not be created')

    return parser.parse_args()


if '__main__' == __name__:
    run()
