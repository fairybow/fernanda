# <img src="fernanda/fernanda.ico" alt="Colorful conch shell icon." width="26px"/> fernanda

## **hi** :wave:

Fernanda is for drafting long-form fiction. (At least, that's the plan.)

<p><kbd><img src="fernanda/docs/screens/gif_1.gif" alt="GIF of Fernanda (window light, editor amber) running on Windows 11. width="360px""/></kbd></p>

## **so?** :zzz:

idk! Fernanda is a personal project, a work-in-progress, and I am (clearly) not a programmer.

I decided I didn't like existing novel-writing software very much, and I wanted something that would be easy to use, lightly-customizable, and distraction-free, for faster, more peaceful drafting.

I like the idea of binder-style writing software, but I feel like a lot of writing programs like that, with features like corkboards or mindmaps or timelines, really just end up keeping me from actually drafting. Plus, they usually tend to take a kind of ownership over my files and drafts. So, I wanted something that didn't do any of those things. I want to draft in Fernanda, and then I want to be able to take that plain-text draft and move it easily to whatever other place or program I want or need to.

## **what is it made with?** :hammer:

Fernanda is built with Qt & C++ using Visual Studio, and I *barely* know what that means.

## **ew don't touch my files** :floppy_disk:

Only kinda!

I have tried very hard to make the way that Fernanda handles files incomprehensible and scary.

Just know that, basically, all the changes you're making among files are being held separately in Fernanda's data folder (`your-user-data-folder/.fernanda/`).

(It's also possible that this is just how some text editors work. I, of course, would *not* know, because I don't know anything.)

When you open a file from the file pane, a couple of things happen:
- that file's text is opened in the editor, and then that file is *closed*
- an autosave timer begins
- clicking onto a different file (or staying on the same file for the duration of the autosave timer) will save the editor's contents into a *separate* temp file in a mirrored directory created in `your-user-data-folder/.fernanda/.active_temp/`
- the next time you click on that same file (or another file), Fernanda first checks for a temp copy of the file
- if it finds a temp file, it opens that instead of the real file
- it is all a mean trick
- again, the temp file (or real file) is closed after its contents are put into the editor
- so, no files are actually held open in memory, even though you can click from file to file and see your edits persist
- you can open a file (at its true path) that you're editing in Fernanda and see that no changes to that file have actually been made yet.
- you could also delete the real file while you're editing it, but I have no idea what happens when you do that, it prolly isn't good I bet
- when you save your changes, all that happens is: 1) another temp save is performed on the current file; 2) then the temp file is swapped with the real file
- when you perform this save "swap", your original file travels to `your-user-data-folder/.fernanda/backup/.rollback/` and stays there until the next time you save the same file
- when you `Save All` files, Fernanda is just temp-saving your current in-editor file, and then swapping *every* file that is marked as changed (shown via *italics* and an asterisk)
- when you close Fernanda, any remaining (that is, "unsaved") temp files are permanently removed!
- if Fernanda closes abruptly, without going through the proper shutdown process (where you'll be notified of unsaved changes), then temp files are not removed, and you will be prompted to recover them the next time you open Fernanda (or, you can also "physically" grab them from the temp directory, where they will still be)
- ha ha
- you get the picture

## **why does it look all stupid, just like @fairybow?** :eyes:

The look of Fernanda is inspired by (in no particular order):
- [Atom](https://atom.io/)
- [Windows Terminal](https://github.com/Microsoft/Terminal)
- [WordStar 4](https://en.wikipedia.org/wiki/WordStar) (lol)

Fernanda has some fonts and themes available, and it'll also incorporate any font (`.ttf` only, for now) or custom theme files you drop anywhere in the `.fernanda` data folder. Fernanda's themes files are a list of variable names equated to your chosen hex values, and an example file for both window and editor themes will be created in the data folder.

Changes you make (like font, theme, window size, UI toggles) are saved when you make them, and Fernanda will start-up just the way you left it.

## **why should I use it?** :baby:

You prolly shouldn't tbh lmao

Definitely not right now. It isn't nearly done!

## **did u use my code??** :cop:

I may have, and I'm sorry. I tried not to, but I have spent a *lot* of time trawling Stack Overflow and etc. If I accidentally used some novel code you wrote, please let me know, and I would be happy to credit you.