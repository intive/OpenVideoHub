This directory contains source code for the plugins for OpenVideoHub as well as
interface, documentation and an example for creating new plugins.

examplePlugin/
 Example plugin implementation (methods are empty, should be filled by creator).
 Detailed description of each method as well as a short guide what have to be
 modified in this example to get working plugin can be found in Plugin.h or in
 provided html documentation (see html/)

Interface/
 This is the ECOM interface for the plugins. No modifications required.
 Make sure to add this path to your systemincludes in .mmp file of your plugin.

html/
 Contains documentation for the API used by plugins. Generated with dogygen
 from examplePlugin/Plugin.h.

youtube/
 Contains implementaiton of a plugin for YouTube site.

dailymotion/
 Contains implementaiton of a plugin for DailyMotion site.

metacafe/
 Contains implementaiton of a plugin for MetaCafe site.
