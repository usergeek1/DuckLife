Game!!

Log Entries

Wednesday May 18 1pm-3pm
WHO: Saumya
WHAT: Wrote logic for remove_all/ set_moving screen + wrote skeleton for collision forces/game play logic
BUGS: N/A
RESOURCES USED: N/A

Wednesday May 18 2pm-6pm
WHO: Purvi
WHAT: Wrote the logic for duck, ocean, and wall generation.
BUGS: Walls were offset
RESOURCES USED: N/A

Wednesday May 18 4pm-5pm
WHO: Mabel
**WHAT:**Set up repo for game
BUGS: N/A
RESOURCES USED: N/A

Wednesday May 18 1am - 2am
WHO: Purvi
WHAT: Unsuccessfully worked to fix wall offset error
BUGS: Walls were offset
RESOURCES USED: N/A
RESOURCES USED: N/A

Wednesday May 18 1:30am - 4:07am
WHO: Mabel
WHAT: Added code to work with diff scenes of state. Also updated impulse code so that there are no volatile launches
BUGS: N/A
RESOURCES USED: N/A

Wednesday May 18 2am - 2:30am
WHO: Purvi
WHAT: Wrote the logic for an earth-like body underneath the screen to simulate gravity
BUGS: N/A

Thursday May 19 12pm - 1pm
WHO: Purvi
WHAT: Discussed rendering images + sprites
BUGS: None

Thursday May 19 9:00pm-10:15pm
WHO: Saumya
WHAT: Wrote buoyancy force logic + started looking into generating images
BUGS: N/A
RESOURCES USED: N/A

Thursday May 19 9:00pm-10:15pm
WHO: Arushi
WHAT: Created duck sprite graphics
BUGS: N/A
RESOURCES USED: piskelapp.com

Thursday May 18 2- 6 am
WHO: Purvi
WHAT: Wrote a majority of the code for start screen, game screen, and end screen generation
BUGS: None

Friday May 20 3:00AM-4:30AM
WHO: Arushi
WHAT: Updated enums, state struct, main function for updating duck positions, and wrote functions for key presses, adding all collisions, and player bounds + helper functions
BUGS: N/A
RESOURCES USED: N/A

Saturday May 21 10-11:30 AM
WHO: Saumya
WHAT: In preparation for integration meeting with rest of group, pulled code and tried to get it to compile (fixed compilation issues) + edited code for obstacles to fix some dimensions/ edited code for gravity walls force abstraction to get it to compile
BUGS: N/A
RESOURCES USED: N/A

Saturday May 21 12pm-2pm
WHO: Arushi, Purvi, Saumya, Mabel
WHAT: Group integration meeting -- worked on integrating everyone's parts together
BUGS: issue with things appear + putting things together

Wednesday May 25 7:00pm - 7:40pm
WHO: Purvi
WHAT: Designed a moving coin sprite
BUGS: N/A

Wednesday May 25 7:45pm-9:15pm
WHO: Arushi, Purvi, Saumya
WHAT: Worked on fixing buoyancy/gravity, handled changes to forces based on water level
BUGS: wrong constants

Thursday May 26 11:00pm-1:00 AM
WHO: Saumya
WHAT: generated duck sprite image (wrote image generation); updated body to have image field + grated init with image_info, altered sdl_wrapper.c to have image_generation functions
BUGS: collisions were incorrect
RESOURCES USED: N/A

Thursday May 26 2:00am-3:00am
WHO: Arushi
WHAT: Looked into removing forces (tried Dif methods, wrote function for removing last force,  wrote list functions for adding to front / second-to-last of list) [none of this to be used because removing forces is now handled through its incorporation in buoyancy]
BUGS: N/A
RESOURCES USED: N/A

Saturday May 28 2:30pm-4:00pm
WHO: Arushi
WHAT:

Fixed buoyancy force to incorporate applying force depending on above/below water level
Created duck gravity force to account for not applying gravity under water
Incorporated new buoyancy / duck gravity forces properly in demo
Fixed issues with bound collisions (so y position right)
Fixed issues with collisions with obstacles (adjusted elasticity)
Adjusted duck dive/jump velocities and constants for gravity/density to improve duck jumping/diving
Added movement of duck left/right when jumping/diving
BUGS: N/A
RESOURCES USED: N/A


Saturday May 28 11:00pm-11:30 PM
WHO: Saumya
WHAT: Debugged image generation for incorrect collision handling
BUGS: used body_get_centroid in code vs. correct centroid function within sdl_wrapper.c
RESOURCES USED: N/A

Sunday May 29 1pm - 1:30 pm
WHO: Purvi
WHAT: Designed the iceberg sprite
BUGS: N/A

Sunday May 29 6:00pm - 10:30pm
WHO: Purvi
WHAT: Learned how to do mouse clicks and implemented in SDL_wrapper
BUGS: clicks did not work

Sunday May 29 1am - 5am
WHO: Purvi
WHAT: Debugged clicks, implemented onClick and other functions in demo file, and found way to record position of the click (to check if inside a certain area)
BUGS: Very basic form of clicks worked, but was not integrated with the rest of the game

Monday May 30 2pm - 4pm
WHO: Purvi
WHAT: Started connecting the screens together and the clicks to the screens
BUGS: MakeFile error

Tuesday May 31 4:00pm - 6:30pm
WHO: Purvi
WHAT: Finished screen generation, conitinued working on connecting screens together, and connecting the clicks to the screens. Debugged
BUGS: Makefile, exception thrown

Tuesday May 31 7:30 pm - 9:00 pm
WHO: Purvi
WHAT: Fixed errors so got a rudimentary version of screen changes working
BUGS: None

Wednesday June 1 3am - 5am
WHO: Purvi
WHAT: Connected the clicks with the rest of the game
BUGS: None

Wednesday June 1 5pm-7pm
WHO: Saumya
WHAT: Finished text generation for score and text
BUGS: Score disappeared after some time; wasn't showing up initially as well as TTF_Init function was disappearing (this was resolved within the timer period, however)

Wednesday June 1 10pm - 11:30pm
WHO: Purvi
WHAT: End screen click generation and connected to the rest of the game
BUGS: None

Thursday June 2 10:30 AM -1pm
WHO: Saumya
WHAT: Imlemented sound feature (researched into SDL sounds libraries + changed sdl_wrapper.c to have sound_init functions, use sdl_mixer)
BUGS: Was incorrectly obtaining sounds through collisions; fixed within this time period by adding play_sound() function to collision handler functions
RESOURCES USED: SDL_Mixer libraries online

Thursday June 2 4pm - 5:30 pm
WHO: Purvi
WHAT: Created buttons for the different levels and connected to clicks. Debugged
BUGS: None

Friday June 3 5pm-6pm
WHO: Arushi
WHAT: Added game modes + abstracted some parts of the code
BUGS: N/A
RESOURCES USED: N/A

Friday June 3 7-9pm
WHO: Arushi, Mabel, Saumya
WHAT: Arushi - switching screens from start to gameplay to end to start using keyboard functions, finalized game mode stuff and functions ; Mabel - start/end screen images, also worked on switching screens
BUGS: N/A
RESOURCES USED: N/A

Saturday June 4 3pm-5:30pm
WHO: Saumya
WHAT: Resolved issue with score disappearing + resolved issue with score appearing at all times (not what we wanted in our game); called TTF_CloseFont to resolve the first of aforementioned issues + checked for state enum for cur_screen to get score on appropriate pages at the appropriate times + got timer to show up
BUGS: Timer reset after 6 seconds + timer was not resetting back to zero after each game session
RESOURCES USED: N/A

Saturday June 4 11pm-12:30am
WHO: Arushi
WHAT: Made ship and float drawings
BUGS: N/A
RESOURCES USED: making sprite website

Saturday June 4 11pm-1:00 AM
WHO: Saumya and Mabel
WHAT: Resolved issues with timer (w/logic + reset feature by checking for state-> cur_scene field) + added new obstacle images + generated new obstacles + generated background image for gameplay
BUGS: Images were drawn a little too big, so image sizes were redone by arushi
RESOURCES USED: N/A

Sunday June 5 2-2:30 Am
WHO: Arushi
WHAT: Redrew game obstacle drawings to fix/ improve collisions
BUGS: Images were drawn a little too big, so image sizes were redone in this session
RESOURCES USED: N/A

Sunday June 5 10-11:50 AM
WHO: Saumya
WHAT: Added more sounds/ image generation on obstales + altered font + played around with bounds for collision checking
BUGS: N/A
RESOURCES USED: N/A

Sunday June 5 3-4pm
WHO: Arushi
WHAT: Updated positions of obstacles, adjusted constants / elasticities, worked on fixing collisions
BUGS: Issues w duck going through obstacles
RESOURCES USED: N/A

Sunday June 5 10-11:30pm
WHO: Arushi
WHAT: Finalized / fixed code, adjusted duck keyboard to better deal with collisions, added obstacle moving off left (-Mabel)
BUGS: N/A
RESOURCES USED: N/A

Sunday June 5 4pm - 5:30 pm
WHO: Purvi
WHAT: Restored everything that got deleted in merges (all the click stuff and more).
BUGS: Clicks weren't working because of incorrect merging

Sunday June 5 11pm - 12pm
WHO: Purvi
WHAT: Debugged and got clicks to work again. 
BUGS: Current scene was incorrectly set among other errors

Monday June 6 1:30-2am
WHO: Arushi, Purvi
WHAT: Integrated two demos to combine final version from Arushi's  + clicks for start screen from Purvi's
BUGS: N/A
RESOURCES USED: N/A

Monday June 6 2:05-2:30am
WHO: Arushi
WHAT:  Adjusted button start image so rectangles for clicks aligned better
BUGS: N/A
RESOURCES USED:  N/A
