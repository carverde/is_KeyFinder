/*************************************************************************

	Copyright 2011 Ibrahim Sha'ath

	This file is part of KeyFinder.

	KeyFinder is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	KeyFinder is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with KeyFinder.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/

#include "downsampler.h"

#include "downsamplerib.h"
#include "downsamplerlibsrc.h"

Downsampler* Downsampler::getDownsampler(int /* downsampleFactor */, int frameRate, float maxFreq){
  if(frameRate == 44100 && maxFreq < 2205.0){
		return new PrimaryDownsampler();
	}else{
		return new SecretRabbitDownsampler();
	}
}
