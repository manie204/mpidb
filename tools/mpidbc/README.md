# mpidbc

Helper tool that formats output of *mpidb* according to one of the available templates.

## Currently available templates

* txt
  * Easy-to-read plain-text format that can easily be parsed by external tools.
* vscode
  * Launch-configuration file (```launch.json```) for [Visual Studio Code](https://github.com/Microsoft/vscode).
  * To launch the debugging session from within the IDE:
    1. Open a folder via *File->Open Folder* (preferrably the root directory of your source code).
    2. Create a subfolder called ```.vscode``` in the previously opened folder.
    3. Place the ```launch.json``` inside the ```.vscode``` subfolder.
    4. Make sure that you have the necessary debugging extensions (e.g. [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)).
    5. Place breakpoints, etc.
    5. From the debug panel, choose the compound configuration to debug all MPI ranks concurrently.
