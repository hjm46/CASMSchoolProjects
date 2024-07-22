# Jace Martin
# hjm46

# This is used in a few places to make grading your project easier.
.eqv GRADER_MODE 0

# This .include has to be up here so we can use the constants in the variables below.
.include "game_constants.asm"

# ------------------------------------------------------------------------------------------------
.data
# Boolean (0/1): 1 when the game is over, either successfully or not.
game_over: .word 0

# 0 = player can move, nonzero = they can't
player_move_timer: .word 0

# How many diamonds the player has collected.
player_diamonds: .word 0

# How many dirt blocks the player has picked up.
player_dirt: .word 0

# How many bugs the player has saved.
bugs_saved: .word 0

# How many bugs need to be saved.
bugs_to_save: .word 0

# Object arrays. These are parallel arrays. The player object is in slot 0,
# so the "player_x" and "player_y" labels are pointing to the same place as
# slot 0 of those arrays. Same thing for the other arrays.
object_type:   .word OBJ_EMPTY:NUM_OBJECTS
player_x:
object_x:      .word 0:NUM_OBJECTS # fixed 24.8 - X position
player_y:
object_y:      .word 0:NUM_OBJECTS # fixed 24.8 - Y position
player_vx:
object_vx:     .word 0:NUM_OBJECTS # fixed 24.8 - X velocity
player_vy:
object_vy:     .word 0:NUM_OBJECTS # fixed 24.8 - Y velocity
player_moving:
object_moving: .word 0:NUM_OBJECTS # 0 = still, nonzero = moving for this many frames
player_dir:
object_dir:    .word 0:NUM_OBJECTS # direction object is facing

.text

# ------------------------------------------------------------------------------------------------

# these .includes are here to make these big arrays come *after* the interesting
# variables in memory. it makes things easier to debug.
.include "display_2227_0611.asm"
.include "tilemap.asm"
.include "textures.asm"
.include "map.asm"
.include "levels.asm"
.include "obj.asm"
.include "collide.asm"

# ------------------------------------------------------------------------------------------------

.globl main
main:
	# load the map and objects
	la  a0, level_1
	#la  a0, test_level_dirt
	#la  a0, test_level_diamonds
	#la  a0, test_level_vines
	#la  a0, test_level_boulders
	#la  a0, test_level_goal
	#la  a0, test_level_bug_movement
	#la  a0, test_level_bug_vines
	#la  a0, test_level_bug_goal
	#la  a0, test_level_blank
	jal load_map

	# main game loop
	_loop:
		jal update_all
		jal draw_all
		jal display_update_and_clear
		jal wait_for_next_frame
	jal check_game_over
	beq v0, 0, _loop

	# when the game is over, show a message
	jal show_game_over_message
syscall_exit

# ------------------------------------------------------------------------------------------------
# Misc game logic
# ------------------------------------------------------------------------------------------------

# returns a boolean (1/0) of whether the game is over. 1 means it is.
check_game_over:
enter
	# might seem silly to have the whole function be one line,
	# but abstracting it into a function like this means that we
	# can expand the "game over" condition in the future.
	lw v0, game_over
leave

# ------------------------------------------------------------------------------------------------

# does what it says.
show_game_over_message:
enter
	# first clear the display
	jal display_update_and_clear

	# they finished successfully!
	li   a0, 7
	li   a1, 15
	lstr a2, "yay! you"
	li   a3, COLOR_GREEN
	jal  display_draw_colored_text

	li   a0, 12
	li   a1, 21
	lstr a2, "did it!"
	li   a3, COLOR_GREEN
	jal  display_draw_colored_text

	li   a0, 25
	li   a1, 37
	la   a2, tex_diamond
	jal  display_blit_5x5_trans

	li   a0, 32
	li   a1, 37
	lw   a2, player_diamonds
	jal  display_draw_int

	jal display_update_and_clear
leave

# ------------------------------------------------------------------------------------------------

# updates all the parts of the game.
update_all:
enter
	jal obj_update_all
	jal update_timers
	jal update_camera
leave

# ------------------------------------------------------------------------------------------------

# updates all timer variables (well... there's just one)
update_timers:
enter
	lw t0, player_move_timer
	ble t0, 0, _end_if
	sub t0, t0, 1
	sw t0, player_move_timer
	_end_if:
leave

# ------------------------------------------------------------------------------------------------

# positions camera based on player position.
update_camera:
enter
	li a0, 0
	jal obj_get_topleft_pixel_coords
	add a0, v0, CAMERA_OFFSET_X
	add a1, v1, CAMERA_OFFSET_Y
	jal tilemap_set_scroll
leave

# ------------------------------------------------------------------------------------------------
# Player object
# ------------------------------------------------------------------------------------------------

# a0 = object index (but you can just access the player_ variables directly)
obj_update_player:
enter
	lw t0, player_moving
	bne t0, 0, _moving
		jal player_check_goal
		
		jal player_check_vines
		bne v0, 0, _endif
			jal player_check_place_input
			jal player_check_move_input
	j _endif
	_moving:
		li a0, 0
		jal obj_move
	_endif:
	
	jal player_check_dig_input
leave

# ------------------------------------------------------------------------------------------------
# Diamond object
# ------------------------------------------------------------------------------------------------

# a0 = object index
obj_update_diamond:
enter s0
	move s0, a0
	jal obj_move_or_check_for_falling
	move a0, s0
	jal obj_collides_with_player
	bne v0, 1, _return
	
	lw t0, player_diamonds
	add t0, t0, 1
	sw t0, player_diamonds
	move a0, s0
	jal obj_free
	
	_return:	
leave s0

# ------------------------------------------------------------------------------------------------
# Boulder object
# ------------------------------------------------------------------------------------------------

# a0 = object index
obj_update_boulder:
enter
	jal obj_move_or_check_for_falling
leave

# ------------------------------------------------------------------------------------------------
# Bug object
# ------------------------------------------------------------------------------------------------

# a0 = object index
obj_update_bug:
enter s0
	move s0, a0
	
	#checks if bug is on goal
	move a0, s0
	jal obj_get_tile_coords
	
	move a0, v0
	move a1, v1
	jal tilemap_get_tile
	
	bne v0, TILE_GOAL, _continue
	move a0, s0
	jal obj_free
	
	lw t0, bugs_saved
	add t0, t0, 1
	sw t0, bugs_saved
	j _return
	
	_continue:
	lw t0, object_moving(s0)
	beq t0, 0, _start_moving
	move a0, s0
	jal obj_move
	j _return
	
	_start_moving:
	move a0, s0
	jal bug_eat_vines
	
	move a0, s0
	jal bug_move
	
	_return:
leave s0
#-----------------------------------------------------------

bug_eat_vines:
enter s0
	move s0, a0
	
	#checks if on vines
	move a0, s0
	jal obj_get_tile_coords
	
	move a0, v0
	move a1, v1
	jal tilemap_get_tile
	bne v0, TILE_VINES, _endif
	
	#make vines disappear
	move a0, s0
	jal obj_get_tile_coords
	move a0, v0
	move a1, v1
	li a2, TILE_EMPTY
	jal tilemap_set_tile
	
	_endif:
	li v0, 0
leave s0

#--------------------------------------------------------------

bug_move:
enter s0, s1
	move s0, a0
	
	#check if there's something solid in front
	lw t0, object_dir(s0)
	move a0, s0
	move a1, t0
	jal obj_collision_check
	move s1, v0
	
	#check if there's something solid to the left
	lw t0, object_dir(s0)
	add t0, t0, DIR_W
	and t0, t0, 3
	
	move a0, s0
	move a1, t0
	jal obj_collision_check

	beq v0, COLLISION_NONE, _turn_left
	beq s1, COLLISION_NONE, _move_forward
	
	#turn right
	lw t0, object_dir(s0)
	add t0, t0, DIR_E
	and t0, t0, 3
	sw t0, object_dir(s0)
	j _return
	
	_move_forward:
	move a0, s0
	li a1, BUG_MOVE_VELOCITY
	li a2, BUG_MOVE_DURATION
	jal obj_start_moving_forward
	j _return
	
	_turn_left: #and move forward
	lw t0, object_dir(s0)
	add t0, t0, DIR_W
	and t0, t0, 3
	sw t0, object_dir(s0)
	
	move a0, s0
	li a1, BUG_MOVE_VELOCITY
	li a2, BUG_MOVE_DURATION
	jal obj_start_moving_forward
	
	_return:
leave s0, s1

# ------------------------------------------------------------------------------------------------
# Drawing functions
# ------------------------------------------------------------------------------------------------

# draws everything.
draw_all:
enter
	jal tilemap_draw
	jal obj_draw_all
	jal hud_draw
leave

# ------------------------------------------------------------------------------------------------

# draws the HUD ("heads-up display", the icons and numbers at the top of the screen)
hud_draw:
enter
	# draw a big black rectangle - this covers up any objects that move off
	# the top of the tilemap area
	li  a0, 0
	li  a1, 0
	li  a2, 64
	li  a3, TILEMAP_VIEWPORT_Y
	li  v1, COLOR_BLACK
	jal display_fill_rect_fast

	# draw how many diamonds the player has
	li  a0, 1
	li  a1, 1
	la  a2, tex_diamond
	jal display_blit_5x5_trans

	li  a0, 7
	li  a1, 1
	lw  a2, player_diamonds
	jal display_draw_int

	# draw how many dirt blocks the player has
	li  a0, 20
	li  a1, 1
	la  a2, tex_dirt
	jal display_blit_5x5_trans

	li  a0, 26
	li  a1, 1
	lw  a2, player_dirt
	jal display_draw_int

	# draw how many bugs have been saved and need to be saved
	li  a0, 39
	li  a1, 1
	la  a2, tex_bug_N
	jal display_blit_5x5_trans

	li  a0, 45
	li  a1, 1
	lw  a2, bugs_saved
	jal display_draw_int

	li  a0, 51
	li  a1, 1
	li  a2, '/'
	jal display_draw_char

	li  a0, 57
	li  a1, 1
	lw  a2, bugs_to_save
	jal display_draw_int
leave

# ------------------------------------------------------------------------------------------------

# a0 = object index (but you can just access the player_ variables directly)
obj_draw_player:
enter
	jal obj_get_topleft_pixel_coords
	move a0, v0
	move a1, v1
	
	li t0, 0
	lw t0, player_dir(t0)
	mul t0, t0, 4
	lw a2, player_textures(t0)
	jal blit_5x5_sprite_trans
leave

# ------------------------------------------------------------------------------------------------

# a0 = object index
obj_draw_diamond:
enter
	jal obj_get_topleft_pixel_coords
	move a0, v0
	move a1, v1
	
	la a2, tex_diamond
	jal blit_5x5_sprite_trans
leave

# ------------------------------------------------------------------------------------------------

# a0 = object index
obj_draw_boulder:
enter s0
	jal obj_get_topleft_pixel_coords
	move a0, v0
	move a1, v1
	
	la a2, tex_boulder
	jal blit_5x5_sprite_trans
leave s0

# ------------------------------------------------------------------------------------------------

# a0 = object index
obj_draw_bug:
enter s0
	move s0, a0
	jal obj_get_topleft_pixel_coords
	move a0, v0
	move a1, v1
	
	lw t0, object_dir(s0)
	mul t0, t0, 4
	lw a2, bug_textures(t0)
	jal blit_5x5_sprite_trans
leave s0

# ------------------------------------------------------------------------------------------------

# a0 = world x
# a1 = world y
# a2 = pointer to texture
# draws a 5x5 image, but coordinates are relative to the "world" (i.e. the tilemap).
# figures out the screen coordinates and draws it there.
blit_5x5_sprite_trans:
enter
	# draw the dang thing
	# x = x - tilemap_scx + TILEMAP_VIEWPORT_X
	lw  t0, tilemap_scx
	sub a0, a0, t0
	add a0, a0, TILEMAP_VIEWPORT_X

	# y = y - tilemap_scy + TILEMAP_VIEWPORT_Y
	lw  t0, tilemap_scy
	sub a1, a1, t0
	add a1, a1, TILEMAP_VIEWPORT_Y

	jal display_blit_5x5_trans
leave


#-------------------------------------------------

player_check_goal:
enter
	li a0, 0
	jal obj_get_tile_coords
	
	move a0, v0
	move a1, v1
	jal tilemap_get_tile
	
	bne v0, TILE_GOAL, _return
	
	lw t0, bugs_saved
	lw t1, bugs_to_save
	bne t0, t1, _return
	
	li t0, 1
	sw t0, game_over
	
	_return:
leave

#-----------------------------------------------------

player_check_vines:
enter
	#checks if on vines
	li a0, 0
	jal obj_get_tile_coords
	
	move a0, v0
	move a1, v1
	jal tilemap_get_tile
	bne v0, TILE_VINES, _endif
	
	#moves backwards
	li a0, 0
	li a1, PLAYER_MOVE_VELOCITY
	li a2, PLAYER_MOVE_DURATION
	jal obj_start_moving_backward
	li v0, 1	
	
	_endif:
	li v0, 0
leave

#-----------------------------------------------------

player_check_place_input:
enter s0
	#is z being pressed
	jal input_get_keys_pressed
	move s0, v0
	
	and t0, s0, KEY_Z
	beq t0, 0, _endif
	
	#is grader mode 0
	li t0, GRADER_MODE
	bne t0, 0, _endif
	
	#is there any stored dirt
	lw t0, player_dirt
	ble t0, 0, _endif
	
	#is there an object there
	li a0, 0
	li a1, TILE_SIZE
	jal obj_get_pixel_coords_in_front
	
	move a0, v0
	move a1, v1
	jal obj_find_at_position
	bne v0, -1, _endif
	
	#is the tile empty
	li a0, 0
	li a1, OBJ_SIZE
	jal obj_get_tile_coords_in_front
	
	move s0, v0
	move a0, v0
	move a1, v1
	jal tilemap_get_tile
	bne v0, TILE_EMPTY, _endif
	
	#placing dirt
	move a0, s0
	move a1, v1
	li a2, TILE_DIRT
	jal tilemap_set_tile
	
	lw t0, player_dirt
	sub t0, t0, 1
	sw t0, player_dirt
	
	_endif:
leave s0

#-----------------------------------------------------

player_check_move_input:
enter s0
	jal input_get_keys_held
	move s0, v0
	
	#move left
	and t0, s0, KEY_L
	beq t0, 0, _endif_l
	
	li a0, DIR_W
	jal player_try_move
	
	_endif_l:
	
	#move right
	and t0, s0, KEY_R
	beq t0, 0, _endif_r
	
	li a0, DIR_E
	jal player_try_move
	
	_endif_r:
	
	#move up
	and t0, s0, KEY_U
	beq t0, 0, _endif_u
	
	li a0, DIR_N
	jal player_try_move
	
	_endif_u:
	
	#move down
	and t0, s0, KEY_D
	beq t0, 0, _endif_d
	
	li a0, DIR_S
	jal player_try_move
	
	_endif_d:
leave s0

#-----------------------------------------------------

player_try_move:
enter
	#changes directions
	lw t0, player_dir
	beq t0, a0, _change_dir
	sw a0, player_dir
	li t0, PLAYER_MOVE_DELAY
	sw t0, player_move_timer
	
	#check if can move
	_change_dir:
	lw t0, player_move_timer
	bne t0, 0, _return
	li t0, PLAYER_MOVE_DELAY
	sw t0, player_move_timer
	
	li a0, 0
	lw a1, player_dir
	jal obj_collision_check
	
	beq v0, COLLISION_TILE, _return
	beq v0, COLLISION_OBJ, _obj
	j _default
	
	_obj:
	move a0, v1
	jal player_try_push_object
	beq v0, 0, _return
	
	#moves
	_default:
	li a0, 0
	li a1, PLAYER_MOVE_VELOCITY
	li a2, PLAYER_MOVE_DURATION
	jal obj_start_moving_forward
	
	_return:
leave

#--------------------------------------------

player_check_dig_input:
enter s0
	#checks if X is pressed
	jal input_get_keys_pressed
	move s0, v0
	
	and t0, s0, KEY_X
	beq t0, 0, _endif
	
	#checks if tile in front is dirt
	li a0, 0
	li a1, OBJ_SIZE
	jal obj_get_tile_coords_in_front
	
	move s0, v0
	move a0, s0
	move a1, v1
	jal tilemap_get_tile
	bne v0, TILE_DIRT, _endif
	
	#makes dirt disappear
	move a0, s0
	move a1, v1
	li a2, TILE_EMPTY
	jal tilemap_set_tile
	
	#increments dirt
	lw t0, player_dirt
	bge t0, PLAYER_MAX_DIRT, _endif
	add t0, t0, 1
	sw t0, player_dirt
	
	_endif:
	
	
leave s0

#----------------------------------------------

player_try_push_object:
enter s0	
	move s0, a0
	#checks if object is a boulder
	lw t0, object_type(s0)	
	bne t0, OBJ_BOULDER, _return
	
	#player direction
	lw t0, player_dir
	beq t0, DIR_N, _return_no
	beq t0, DIR_S, _return_no
	
	move s0, a0
	#is boulder moving
	lw t0, object_moving(s0)
	bne t0, 0, _return_no
	
	#is there a tile in front
	lw t0, player_dir
	sw t0, object_dir(s0)
	move a0, s0
	jal obj_get_tile_coords_in_front
	
	move a0, v0
	move a1, v1
	jal tilemap_get_tile
	
	bne v0, TILE_EMPTY, _return_no
	
	#is there a boulder in front
	lw t0, player_dir
	sw t0, object_dir(s0)
	move a0, s0
	li a1, TILE_SIZE
	jal obj_get_pixel_coords_in_front
	
	move a0, v0
	move a1, v1
	jal obj_find_at_position
	
	beq v0, -1, _continue
	lw t0, object_type(v0)
	beq t0, OBJ_BOULDER, _return_no
	
	_continue:
	
	_push:
	move a0, s0
	lw a1, player_dir
	jal obj_push
	
	_return_yes:
	li v0, 1
	j _return
	
	_return_no:
	li v0, 0
	
	_return:
	
leave s0

#----------------------------------------------------