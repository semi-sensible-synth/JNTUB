# Production files

Production file generation procedure for JLCPCB.

- Ensure you have the ["Fabrication Toolkit"](https://github.com/bennymeg/JLC-Plugin-for-KiCad) plugin installed in KiCad.
- Load the PCB in KiCad PCBnew.
- Run DRC, ensure there are no errors. (Exclude any acceptable silkscreen overlap issues and missing ground/+5V/-5 connections _between_ boards)
- Select and delete the left 'main' board, click the "Fabrication toolkit" icon in the toolbar to export.
- Ctrl-Z Undo (or reload the PCB), select and delete the left 'control' board, click the "Fabrication toolkit" icon in the toolbar to export.
- Rearrange files following the existing file structure, and unzip gerbers before committing with git.
  - A folder `production` will contain two `JNTUB*` folders.
  - Create a folder `production/{version}` based on the current PCB version.
  - Rename the two `JNTUB*` folders to `control` and `main`, move them into the `production/{version}` folder.
  - Create `production/{version}/control/gerbers` and `production/{version}/main/gerbers` - unzip the `JNTUB.zip` 
    gerber bundles into their respective folders.
  - Delete the `JNTUB.zip` files _(You might want to upload the zips to JLC for production now before deleting)_
    - (It's _probably_ better to have gerbers unzipped in Github to take advantage of change tracking and external online viewers that may not support zipped gerbers).
