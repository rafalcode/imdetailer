# imdetailer
Stands for IMage Detailer, a set of c programs
to take large digital images and focus on a detail
by creating a new image with a reduced version of the large image n the
top left, and textual annotation on the top right and at the bottom
the detail.

This can be done manually to a very high level of quality on a one-by-one basis quite judiciously with any number of programs
that do digital imaging editing. But if the number of these images starts to increase, the task
becomes quickly becomes time consuming to an exasperating level.

Enter imdetailer, takes on this task on a batch level, so that many hundreds and thousands of images can be detailed
in one go. Quality if compromised without a doubt, but sometimes it may be at an acceptable.
Also there is a manual element that can be introduced, by recording a text file specifying the point of interest or the centre of the detail.

# Motivation
Digital photos are everywhere now, people take very very many of them.
Sometimes they have details that can be singled out, but these are often missed.
So this repo is an effort to make the most of past digital photos.

# use cases
Photographs of coastlines from aeroplanes. It's very hard to identify where you are
from the general picture. If you are able to focus on a detail, the issue
is not solved, butg it can often be easier.

I was going to call it visual annotator or visual commentator
not bad either, but I settled for image detailer or imdetailer
because actually annotating is quite manual actually.

so the output images are going to be prefixed with "imda_"
the a standing for automatic and "imdm_" for 
the idea being that the imda_ versions will use file name
exif date and 

But on which detail should be 

# EXIF Orientation
Rather than a field, this seems to be an instruction, and operates on the natural orientation of the image.
It may be imposed by the camera, depending on the type of camera. Especially OpenCV (cv2) imshow
and feh will obey this instruction, so cc1.py now picks it out and rotates (again) the image back.

However when using cv2 imshow, the window is taking on the size of the whole image, which blocks the current cursor position
display which is on the bottom left. Neverthless, the click position goes well into the conft file.

TODO
- need to clean up cod
- produce a text based log file as it will be tedious going through all the _imd.jpg files.
