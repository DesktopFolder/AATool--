import zipfile
import urllib.request
import os

FONT_SOURCE = "https://github.com/IdreesInc/Minecraft-Font/releases/download/v1.0/Minecraft.otf"
BOLD_FONT_SOURCE = "https://github.com/IdreesInc/Minecraft-Font/releases/download/v1.0/Minecraft-Bold.otf"

# Directory should always exist due to custom/ subdir.
os.chdir('az')

def rm(p):
    import shutil
    if os.path.isdir(p):
        shutil.rmtree(p)

def clean_ctm():
    """
    az/ctm/
      ctm_aatool_.../
        assets/
          ...
    """
    import shutil
    print('Cleaning up CTM assets.')
    rt = f'ctm/{os.listdir("ctm")[0]}'
    at = f'{rt}/assets'
    shutil.move(f'{at}/sprites', 'ctm/sprites')
    rm(rt)


def download_url_to(url: str, dest: str):
    print(f'Downloading URL: {url}')
    with urllib.request.urlopen(url) as f:
        with open(dest, 'wb') as file:
            file.write(f.read())
    print(f'Finished downloading URL: {url}')


def get_fonts():
    print('Downloading fonts.')
    if not os.path.isdir('fonts'):
        os.mkdir('fonts')
    download_url_to(FONT_SOURCE, 'fonts/minecraft.otf')
    download_url_to(BOLD_FONT_SOURCE, 'fonts/minecraft-bold.otf')
    print('Finished downloading fonts.')


def do_downloads():
    ver = '1.16'
    loc = f'{ver}.items.zip'
    if not os.path.isfile(loc):
        print('Downloading nerothe (item) assets.')
        with urllib.request.urlopen(f"https://nerothe.com/img/{ver}/items.zip") as f:
            with open(loc, 'wb') as file:
                file.write(f.read())
            print('Download finished. Extracting nerothe.')
            if os.path.isdir('items'):
                print(f'Removing previous items directory to overwrite.')
                os.rmdir('items')
            zipfile.ZipFile(loc).extractall('items')

    loc = 'ctm'
    if not os.path.isdir(loc):
        print('Downloading CTM (misc/extra/criteria) assets.')
        with urllib.request.urlopen('https://github.com/DarwinBaker/AATool/releases/download/v1.7.5.0-release/ctm_aatool_1.7.5.0.zip') as f:
            with open(f'{loc}.zip', 'wb') as file:
                file.write(f.read())
            print('Download finished. Extracting ctm.')
            zipfile.ZipFile(f'{loc}.zip').extractall(loc)
        clean_ctm()


if __name__ == '__main__':
    do_downloads()
    get_fonts()
