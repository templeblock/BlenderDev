# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8-80 compliant>

import bpy
from bpy.types import Operator


def randomize_selected(seed, delta, loc, rot, scale, scale_even):

    import random
    from random import uniform
    from mathutils import Vector

    random.seed(seed)

    def rand_vec(vec_range):
        return Vector(uniform(-val, val) for val in vec_range)

    for obj in bpy.context.selected_objects:

        if loc:
            if delta:
                obj.delta_location += rand_vec(loc)
            else:
                obj.location += rand_vec(loc)
        else:  # otherwise the values change under us
            uniform(0.0, 0.0), uniform(0.0, 0.0), uniform(0.0, 0.0)

        if rot:  # TODO, non euler's
            vec = rand_vec(rot)
            if delta:
                obj.delta_rotation_euler[0] += vec[0]
                obj.delta_rotation_euler[1] += vec[1]
                obj.delta_rotation_euler[2] += vec[2]
            else:
                obj.rotation_euler[0] += vec[0]
                obj.rotation_euler[1] += vec[1]
                obj.rotation_euler[2] += vec[2]
        else:
            uniform(0.0, 0.0), uniform(0.0, 0.0), uniform(0.0, 0.0)

        if scale:
            if delta:
                org_sca_x, org_sca_y, org_sca_z = obj.delta_scale
            else:
                org_sca_x, org_sca_y, org_sca_z = obj.scale

            if scale_even:
                sca_x = sca_y = sca_z = uniform(scale[0], - scale[0])
                uniform(0.0, 0.0), uniform(0.0, 0.0)
            else:
                sca_x, sca_y, sca_z = rand_vec(scale)

            if scale_even:
                aX = -(sca_x * org_sca_x) + org_sca_x
                aY = -(sca_x * org_sca_y) + org_sca_y
                aZ = -(sca_x * org_sca_z) + org_sca_z
            else:
                aX = sca_x + org_sca_x
                aY = sca_y + org_sca_y
                aZ = sca_z + org_sca_z

            if delta:
                obj.delta_scale = aX, aY, aZ
            else:
                obj.scale = aX, aY, aZ
        else:
            uniform(0.0, 0.0), uniform(0.0, 0.0), uniform(0.0, 0.0)


from bpy.props import IntProperty, BoolProperty, FloatVectorProperty


class RandomizeLocRotSize(Operator):
    '''Randomize objects loc/rot/scale'''
    bl_idname = "object.randomize_transform"
    bl_label = "Randomize Transform"
    bl_options = {'REGISTER', 'UNDO'}

    random_seed = IntProperty(
            name="Random Seed",
            description="Seed value for the random generator",
            min=0,
            max=1000,
            default=0,
            )
    use_delta = BoolProperty(
            name="Transform Delta",
            description=("Randomize delta transform values "
                         "instead of regular transform"),
            default=False,
            )
    use_loc = BoolProperty(
            name="Randomize Location",
            description="Randomize the location values",
            default=True,
            )
    loc = FloatVectorProperty(
            name="Location",
            description=("Maximun distance the objects "
                         "can spread over each axis"),
            min=-100.0,
            max=100.0,
            default=(0.0, 0.0, 0.0),
            subtype='TRANSLATION',
            )
    use_rot = BoolProperty(
            name="Randomize Rotation",
            description="Randomize the rotation values",
            default=True,
            )
    rot = FloatVectorProperty(
            name="Rotation",
            description="Maximun rotation over each axis",
            min=-180.0,
            max=180.0,
            default=(0.0, 0.0, 0.0),
            subtype='TRANSLATION',
            )
    use_scale = BoolProperty(
            name="Randomize Scale",
            description="Randomize the scale values",
            default=True,
            )
    scale_even = BoolProperty(
            name="Scale Even",
            description="Use the same scale value for all axis",
            default=False,
            )

    '''scale_min = FloatProperty(
            name="Minimun Scale Factor",
            description="Lowest scale percentage possible",
            min=-1.0, max=1.0, precision=3,
            default=0.15,
            )'''

    scale = FloatVectorProperty(
            name="Scale",
            description="Maximum scale randomization over each axis",
            min=-100.0,
            max=100.0,
            default=(0.0, 0.0, 0.0),
            subtype='TRANSLATION',
            )

    def execute(self, context):
        from math import radians

        seed = self.random_seed

        delta = self.use_delta

        loc = None if not self.use_loc else self.loc
        rot = None if not self.use_rot else self.rot * radians(1.0)
        scale = None if not self.use_scale else self.scale

        scale_even = self.scale_even
        #scale_min = self.scale_min

        randomize_selected(seed, delta, loc, rot, scale, scale_even)

        return {'FINISHED'}
