
# Construct the camera mounting matrix

Camera vector v is (0, 0, distance_from_range_meter)
It has angles pan and depr

Body_x = cos(depr) * cos(pan)
Body_y = -sin(pan) * cos(depr)
Body_z = -cos(depr)

R_cam<-body = (
  cos(depr) * cos(pan),
  -sin(pan) * cos(depr),
  -cos(depr)
)

I should do R_cam * vector. Something is missing. I cant multiply vector by vector.

Transform matrix should be 3x3. How did i calculate those formular. One more try

To convert from camera to body i should apply transformation matrixes. BUT!

Camera rotates only around two axes. Around Y and Z. Pan is positive to the right while our axes dictate positive in different direction. With that being said - i should invert the angle.

I will take the default rotation matrix formula and call it R. We are saying that Rz(-pan) describes the rotation for the pan. Then we gonna rotate similar way around the Y. We take the formula Ry(depr). The directoin of depressoin coincides with the y positive rotation. We could actually take any body axes order we wanted. Transformation would not be too much easier. 

I found the right rotations. Now i need to find out the order. I dont know to be honest. I hope you will correct me. I will use just pan -> depr(where i should find out what order to use? though i feel like in this case no difference at all).

v_body = Rz(-pan) * Ry(depr) * v

Gotta calculate it from the right, not from the left(AFAIK).

There we obtained the body coordinates. These are not ENU yet. We should continue implying the transformations. Now we should do the body->enu.

We know pitch roll and yaw. We should imply roll -> pitch -> yaw in exactly this order(standard for the aviation). But what about the axes. We should align them actually. Or maybe not. Next thing i would do

v_enu = Rroll(roll) * Rpitch(pitch) * Ryaw(yaw) * v_body

---

Let me correct myself after your explanations. I see why i need Rbody<-camera.
Its gonna be
(
  0, 0, 1
  -1, 0, 0
  0, -1, 0
)

Tell me if this is a valid option.

I have v_body. I need v_enu. I say that i first convert the axes.

I take R_0 = 
(
  0 -1 0
  1 0 0
  0 0 1
)

By this i say that i align the v_body with the enu. But no rotation happened yet. Now i say. Ive been given 90 degrees east. Which is -90 degrees rotation by Z. Been given the Pitch 30 which is 30 degrees rotation by x. And roll would be positive to the right.

Now i can create rotation matrix and do this - R_z(-90) * R_y(0) * R_x(30) * R_0 * b_body. And this is how i obtain the enu. IS THAT SO?
