'''
Template for creating new growroom apps
'''
from __future__ import print_function
import errno
import jinja2
import os
import re
import subprocess
from subprocess import call

def ensure_dir(path):
    '''
    mkdir -p for python
    '''
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise

def get_yobuild(g_speak_version):
    '''
    Look up which yobuild this gspeak depends on
    '''
    p = re.compile(r'YOBUILD_PREFIX.*"(.*)"')
    with open("/opt/oblong/g-speak%s/include/libLoam/c/ob-vers-gen.h" % g_speak_version, 'r') as f:
        for line in f:
            m = p.search(line)
            if (m):
                return m.group(1)

    # If g-speak wasn't installed, fall back to asking obs
    # NOTE: if the following line fails, please install the Oblong obs package
    return "/opt/oblong/deps-64-" + subprocess.check_output(["obs", "yovo2yoversion", g_speak_version]).rstrip()

def get_cef_branch(g_speak_version):
    '''
    Look up which cef this g-speak's webthing depends on
    '''
    p = re.compile(r'cefbranch=(.*)')
    with open("/opt/oblong/g-speak%s/lib/pkgconfig/libWebThing.pc" % g_speak_version, 'r') as f:
        for line in f:
            m = p.search(line)
            if (m):
                return m.group(1)

    # If webthing wasn't installed, fall back to asking obs
    # NOTE: if the following line fails, please install the Oblong obs package
    return subprocess.check_output(["obs", "yovo2cefversion", g_speak_version]).rstrip()

def growroom_version(g_speak_home):
    env = os.environ
    old_pcpath = env.get("PKG_CONFIG_PATH", "")
    new_pcpath = "{}:{}".format(os.path.join(g_speak_home, "lib", "pkgconfig"),
                                old_pcpath)
    env["PKG_CONFIG_PATH"] = new_pcpath
    version = subprocess.check_output(["pkg-config", "--modversion",
                                       "libGrowroom"], env=env)
    version = [int(x) for x in version.split(".")]
    version_num = version[0] * 10000 + version[1] * 100 + version[2]
    print("libGrowroom version is", version_num)
    return version_num

def make_name_valid_symbol(name):
    '''
    Make NAME into a valid C++ symbol name
    '''
    return name.lstrip("0123456789-").replace('-', '_')


def obi_new(**kwargs):
    '''
    obi new growroom project_name --gspeak=g_speak_home
    '''

    kwargs['yobuild'] = get_yobuild(kwargs['g_speak_version'])
    kwargs['cef_branch'] = get_cef_branch(kwargs['g_speak_version'])

    project_name = kwargs['project_name']
    project_name_symbol = make_name_valid_symbol(project_name)

    if project_name != project_name_symbol:
        print("Warning: using " + str(project_name_symbol) + " instead of " + str(project_name) + " for generated code")

    trips = [
        (os.path.join("debian", "changelog"), "changelog", {}),
        (os.path.join("debian", "compat"), "compat", {}),
        (os.path.join("share", project_name, "config.protein"),
         "config.protein", {}),
        (os.path.join("debian", "control"), "control", {}),
        (".gitignore", "gitignore", {}),
        (os.path.join("src", "main.cpp"), "main.cpp", {}),
        (os.path.join("src", "AutoRotater.h"), "AutoRotater.h", {}),
        (os.path.join("src", "CountryBorders.h"), "CountryBorders.h", {}),
        (os.path.join("src", "GreenWebThing.h"), "GreenWebThing.h", {}),
        (os.path.join("src", "PushHandler.h"), "PushHandler.h", {}),
        (os.path.join("src", "PushHandler.cpp"), "PushHandler.cpp", {}),
        (os.path.join("src", "SeismoUtils.h"), "SeismoUtils.h", {}),
        (os.path.join("src", "Swiveler.h"), "Swiveler.h", {}),
        (os.path.join("src", "Swiveler.cpp"), "Swiveler.cpp", {}),
        (os.path.join("scripts", "index.js"), "index.js", {}),
        (os.path.join("scripts", "local-repl.js"), "local-repl.js", {'mode': 0755}),
        ("{0}.yaml".format(project_name_symbol), "binding.yaml", {}),
        ("{0}.sublime-project".format(project_name), "proj.sublime-project", {}),
        (os.path.join(".vscode", "c_cpp_properties.json"), "c_cpp_properties.json", {}),
        ("project.yaml", "project.yaml", {}),
        ("package.json", "package.json", {}),
        ("README.md", "README.md", {}),
        (os.path.join("debian", "rules"), "rules", {}),
        ("set-gspeak.sh", "set-gspeak.sh", {'mode': 0755}),
        ("g-speak.dat", "g-speak.dat", {}),
        ("set-version.sh", "set-version.sh", {'mode': 0755}),
        ("three-feld.protein", "three-feld.protein", {}),
        ("three-screen.protein", "three-screen.protein", {}),
        (os.path.join("share", "3-site.protein"), "3-site.protein", {}),
        ("CMakeLists.txt", "CMakeLists.txt", {})]
    env = jinja2.Environment(loader=jinja2.PackageLoader(__name__),
                             keep_trailing_newline=True)

    jinja_vars = kwargs
    g_speak_home = jinja_vars["g_speak_home"]
    jinja_vars["growroom_version"] = growroom_version(g_speak_home)
    jinja_vars["project_name_symbol"] = project_name_symbol
    project_path = kwargs['project_path']
    for file_path, template_name, opts in trips:
        file_path = os.path.join(project_path, file_path)
        ensure_dir(os.path.dirname(file_path))
        # look for the template in any of the envs
        # break as soon as we find it
        try:
            template = env.get_template(template_name)
            with open(file_path, 'w+') as fil:
                fil.write(template.render(jinja_vars))

            if opts.has_key('mode'):
                os.chmod(file_path, opts['mode'])

        except jinja2.TemplateNotFound:
            print("Warning: Could not find template {0}".format(template_name))
    os.chdir(project_path)

    # if npm exists, install the things
    try:
        call(["npm", "install"])
    except OSError as e:
        if (e.errno == errno.ENOENT):
            print("Warning: npm is not installed, so Growroom dependencies "
                  "weren't installed.")
            print("Install npm (https://www.npmjs.com/) and run `npm install`"
                  "to use this project.")
        else:
            raise e
    return 0
