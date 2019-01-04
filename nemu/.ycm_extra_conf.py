import os

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))

include_dirs = [
    './include',
    './tools/qemu-diff/include',
]


flags = [
    '-DEBUG',
    '-DIFF_TEST',
    '-std=c11',
]

def FlagsForFile( filename, **kwargs ):
    """
    Given a source file, retrieves the flags necessary for compiling it.
    """
    for dir in include_dirs:
        flags.append('-I' + os.path.join(CURRENT_DIR, dir))

    return { 'flags': flags }
