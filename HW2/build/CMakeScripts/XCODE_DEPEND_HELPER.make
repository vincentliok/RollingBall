# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.RotateCubeNew.Debug:
/Users/vincentliok/Documents/NYU/Interactive\ Computer\ Graphics/Rotate-Cube-New/build/Debug/RotateCubeNew:
	/bin/rm -f /Users/vincentliok/Documents/NYU/Interactive\ Computer\ Graphics/Rotate-Cube-New/build/Debug/RotateCubeNew


PostBuild.RotateCubeNew.Release:
/Users/vincentliok/Documents/NYU/Interactive\ Computer\ Graphics/Rotate-Cube-New/build/Release/RotateCubeNew:
	/bin/rm -f /Users/vincentliok/Documents/NYU/Interactive\ Computer\ Graphics/Rotate-Cube-New/build/Release/RotateCubeNew


PostBuild.RotateCubeNew.MinSizeRel:
/Users/vincentliok/Documents/NYU/Interactive\ Computer\ Graphics/Rotate-Cube-New/build/MinSizeRel/RotateCubeNew:
	/bin/rm -f /Users/vincentliok/Documents/NYU/Interactive\ Computer\ Graphics/Rotate-Cube-New/build/MinSizeRel/RotateCubeNew


PostBuild.RotateCubeNew.RelWithDebInfo:
/Users/vincentliok/Documents/NYU/Interactive\ Computer\ Graphics/Rotate-Cube-New/build/RelWithDebInfo/RotateCubeNew:
	/bin/rm -f /Users/vincentliok/Documents/NYU/Interactive\ Computer\ Graphics/Rotate-Cube-New/build/RelWithDebInfo/RotateCubeNew




# For each target create a dummy ruleso the target does not have to exist
