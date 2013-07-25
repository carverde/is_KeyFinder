/*************************************************************************

  Copyright 2011-2013 Ibrahim Sha'ath

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

#include "avfilemetadata.h"

const QString emptyString = "";
const char* keyXiphTagComment      = "COMMENT";
const char* keyId3TagiTunesComment = "COMM";
const char* lngId3TagiTunesComment = "eng"; // will this mess up localisations?
const char* keyMp4TagGrouping      = "\251grp";
const char* keyAsfTagGrouping      = "WM/ContentGroupDescription";
const char* keyApeTagGrouping      = "Grouping";
const char* keyId3TagGrouping      = "TIT1";
const char* keyId3TagKey           = "TKEY";
const char* keyMp4TagKey           = "----:com.apple.iTunes:initialkey";
const char* keyXiphTagKey          = "INITIALKEY";
const char* keyAsfTagKey           = "WM/InitialKey";

AVFileMetadata::AVFileMetadata(TagLib::FileRef* inFr, TagLib::File* f) : fr(inFr), genericFile(f) { }
NullFileMetadata::NullFileMetadata      (TagLib::FileRef* fr, TagLib::File* g)                              : AVFileMetadata     (fr, g)       { }
FlacFileMetadata::FlacFileMetadata      (TagLib::FileRef* fr, TagLib::File* g, TagLib::FLAC::File* s)       : AVFileMetadata     (fr, g)       { specificFile = s; }
MpegID3FileMetadata::MpegID3FileMetadata(TagLib::FileRef* fr, TagLib::File* g, TagLib::MPEG::File* s)       : AVFileMetadata     (fr, g)       { specificFile = s; }
AiffID3FileMetadata::AiffID3FileMetadata(TagLib::FileRef* fr, TagLib::File* g, TagLib::RIFF::AIFF::File* s) : MpegID3FileMetadata(fr, g, NULL) { specificFile = s; }
WavID3FileMetadata::WavID3FileMetadata  (TagLib::FileRef* fr, TagLib::File* g, TagLib::RIFF::WAV::File* s)  : AiffID3FileMetadata(fr, g, NULL) { specificFile = s; }
Mp4FileMetadata::Mp4FileMetadata        (TagLib::FileRef* fr, TagLib::File* g, TagLib::MP4::File* s)        : AVFileMetadata     (fr, g)       { specificFile = s; }
AsfFileMetadata::AsfFileMetadata        (TagLib::FileRef* fr, TagLib::File* g, TagLib::ASF::File* s)        : AVFileMetadata     (fr, g)       { specificFile = s; }
ApeFileMetadata::ApeFileMetadata        (TagLib::FileRef* fr, TagLib::File* g, TagLib::APE::File* s)        : AVFileMetadata     (fr, g)       { specificFile = s; }

AVFileMetadata::~AVFileMetadata() { delete fr; }
NullFileMetadata::~NullFileMetadata() { }

// ================================= GENERIC ===================================

QString AVFileMetadata::getByTagEnum(metadata_tag_t tag) const {
  if      (tag == METADATA_TAG_TITLE)    return getTitle();
  else if (tag == METADATA_TAG_ARTIST)   return getArtist();
  else if (tag == METADATA_TAG_ALBUM)    return getAlbum();
  else if (tag == METADATA_TAG_COMMENT)  return getComment();
  else if (tag == METADATA_TAG_GROUPING) return getGrouping();
  else if (tag == METADATA_TAG_KEY)      return getKey();
  return emptyString;
}

QString AVFileMetadata::getTitle() const {
  TagLib::String out = genericFile->tag()->title();
  return QString::fromUtf8(out.toCString(true));
}

QString AVFileMetadata::getArtist() const {
  TagLib::String out = genericFile->tag()->artist();
  return QString::fromUtf8(out.toCString(true));
}

QString AVFileMetadata::getAlbum() const {
  TagLib::String out = genericFile->tag()->album();
  return QString::fromUtf8(out.toCString(true));
}

QString AVFileMetadata::getComment() const {
  TagLib::String out = genericFile->tag()->comment();
  return QString::fromUtf8(out.toCString(true));
}

QString AVFileMetadata::getGrouping() const {
  return GuiStrings::getInstance()->notApplicable();
}

QString AVFileMetadata::getKey() const {
  return GuiStrings::getInstance()->notApplicable();
}

MetadataWriteResult AVFileMetadata::writeKeyToMetadata(
  KeyFinder::key_t key,
  const Preferences& prefs
) {
  MetadataWriteResult result;
  QString data = prefs.getKeyCode(key);
  QString empty;
  for (unsigned int i = 0; i < METADATA_TAG_T_COUNT; i++) {
    result.newTags.push_back(empty);
    if ((metadata_tag_t)i == METADATA_TAG_KEY) {
      // Key field in ID3 holds only 3 chars
      writeKeyByTagEnum(data.left(3), (metadata_tag_t)i, result, prefs);
    } else {
      writeKeyByTagEnum(data, (metadata_tag_t)i, result, prefs);
    }
  }
  return result;
}

void AVFileMetadata::writeKeyByTagEnum(
  const QString& data,
  metadata_tag_t tag,
  MetadataWriteResult& result,
  const Preferences& prefs
) {
  QString delim = prefs.getMetadataDelimiter();
  metadata_write_t write = prefs.getMetadataWriteByTagEnum(tag);
  if (write == METADATA_WRITE_OVERWRITE) {
    if (getByTagEnum(tag) != data && setByTagEnum(data, tag))
      result.newTags[tag] = data;
  } else if (write == METADATA_WRITE_PREPEND) {
    QString current = getByTagEnum(tag);
    QString newData = (current.isEmpty() ? data : data + delim + current);
    if (current.left(data.length()) != data && setByTagEnum(newData, tag))
      result.newTags[tag] = newData ;
  } else if (write == METADATA_WRITE_APPEND) {
    QString current = getByTagEnum(tag);
    QString newData = (current.isEmpty() ? data : current + delim + data);
    if (current.right(data.length()) != data && setByTagEnum(newData, tag))
      result.newTags[tag] = newData;
  }
}

bool AVFileMetadata::setByTagEnum(const QString& data, metadata_tag_t tag) {
  if      (tag == METADATA_TAG_TITLE)    return setTitle(data);
  else if (tag == METADATA_TAG_ARTIST)   return setArtist(data);
  else if (tag == METADATA_TAG_ALBUM)    return setAlbum(data);
  else if (tag == METADATA_TAG_COMMENT)  return setComment(data);
  else if (tag == METADATA_TAG_GROUPING) return setGrouping(data);
  else if (tag == METADATA_TAG_KEY)      return setKey(data);
  return false;
}

bool AVFileMetadata::setTitle(const QString& tit) {
  genericFile->tag()->setTitle(
    TagLib::String(tit.toUtf8().constData(), TagLib::String::UTF8)
  );
  genericFile->save();
  return true;
}

bool AVFileMetadata::setArtist(const QString& art) {
  genericFile->tag()->setArtist(
    TagLib::String(art.toUtf8().constData(), TagLib::String::UTF8)
  );
  genericFile->save();
  return true;
}

bool AVFileMetadata::setAlbum(const QString& alb) {
  genericFile->tag()->setAlbum(
    TagLib::String(alb.toUtf8().constData(), TagLib::String::UTF8)
  );
  genericFile->save();
  return true;
}

bool AVFileMetadata::setComment(const QString& cmt) {
  genericFile->tag()->setComment(
    TagLib::String(cmt.toUtf8().constData(), TagLib::String::UTF8)
  );
  genericFile->save();
  return true;
}

bool AVFileMetadata::setGrouping(const QString& /*grp*/) {
  return false;
}

bool AVFileMetadata::setKey(const QString& /*key*/) {
  return false;
}

// =================================== NULL ====================================

QString NullFileMetadata::getByTagEnum(metadata_tag_t /*tag*/) const {
  return GuiStrings::getInstance()->notApplicable();
}

QString NullFileMetadata::getTitle() const {
  return GuiStrings::getInstance()->notApplicable();
}

QString NullFileMetadata::getArtist() const {
  return GuiStrings::getInstance()->notApplicable();
}

QString NullFileMetadata::getAlbum() const {
  return GuiStrings::getInstance()->notApplicable();
}

QString NullFileMetadata::getComment() const {
  return GuiStrings::getInstance()->notApplicable();
}

bool NullFileMetadata::setTitle(const QString& /*tit*/) {
  return false;
}

bool NullFileMetadata::setArtist(const QString& /*tit*/) {
  return false;
}

bool NullFileMetadata::setAlbum(const QString& /*tit*/) {
  return false;
}

bool NullFileMetadata::setComment(const QString& /*cmt*/) {
  return false;
}

// =================================== FLAC ====================================

QString FlacFileMetadata::getComment() const {
  // TagLib's default behaviour treats Description as Comment
  if (specificFile->xiphComment()->contains(keyXiphTagComment)) {
    TagLib::String out = specificFile->xiphComment()->fieldListMap()[keyXiphTagComment].toString();
    return QString::fromUtf8((out.toCString(true)));
  } else {
    return emptyString;
  }
}

QString FlacFileMetadata::getKey() const {
  TagLib::Ogg::XiphComment* c = specificFile->xiphComment();
  if (!c->fieldListMap().contains(keyXiphTagKey))
    return emptyString;
  TagLib::String out = c->fieldListMap()[keyXiphTagKey].toString();
  return QString::fromUtf8((out.toCString(true)));
}

bool FlacFileMetadata::setComment(const QString& cmt) {
  // TagLib's default behaviour treats Description as Comment
  specificFile->xiphComment()->addField(
    keyXiphTagComment,
    TagLib::String(cmt.toUtf8().constData(), TagLib::String::UTF8),
    true
  );
  genericFile->save();
  return true;
}

bool FlacFileMetadata::setKey(const QString& key) {
  specificFile->xiphComment()->addField(
    keyXiphTagKey,
    TagLib::String(key.toUtf8().constData(), TagLib::String::UTF8),
    true
  );
  specificFile->save();
  return true;
}

// =================================== MPEG ====================================

QString MpegID3FileMetadata::getGrouping() const {
  return getGroupingId3v2(specificFile->ID3v2Tag());
}

QString MpegID3FileMetadata::getGroupingId3v2(const TagLib::ID3v2::Tag* tag) const {
  if (tag->isEmpty()) // ID3v1 doesn't support the Grouping tag
    return GuiStrings::getInstance()->notApplicable();
  if (!tag->frameListMap().contains(keyId3TagGrouping))
    return emptyString;
  TagLib::ID3v2::FrameList l = tag->frameListMap()[keyId3TagGrouping];
  TagLib::String out = l.front()->toString();
  return QString::fromUtf8((out.toCString(true)));
}

QString MpegID3FileMetadata::getKey() const {
  return getKeyId3v2(specificFile->ID3v2Tag());
}

QString MpegID3FileMetadata::getKeyId3v2(const TagLib::ID3v2::Tag* tag) const {
  if (tag->isEmpty()) // ID3v1 doesn't support the Key tag
    return GuiStrings::getInstance()->notApplicable();
  if (!tag->frameListMap().contains(keyId3TagKey))
    return emptyString;
  TagLib::ID3v2::FrameList l = tag->frameListMap()[keyId3TagKey];
  TagLib::String out = l.front()->toString();
  return QString::fromUtf8((out.toCString(true)));
}

bool MpegID3FileMetadata::setComment(const QString& cmt) {
  if (specificFile->ID3v2Tag()->isEmpty()) {
    // TagLib's default behaviour will write a v2 ID3 tag where none exists
    specificFile->ID3v1Tag()->setComment(
      TagLib::String(cmt.toUtf8().constData(), TagLib::String::UTF8)
    );
    specificFile->save(TagLib::MPEG::File::ID3v1, false);
    return true;
  } else {
    // basic tag
    genericFile->tag()->setComment(
      TagLib::String(cmt.toUtf8().constData(), TagLib::String::UTF8)
    );
    // iTunes comment hack
    setITunesCommentId3v2(specificFile->ID3v2Tag(), cmt);
    specificFile->save(
      TagLib::MPEG::File::ID3v2,
      false,
      specificFile->ID3v2Tag()->header()->majorVersion()
    );
  }
  return true;
}

void MpegID3FileMetadata::setITunesCommentId3v2(TagLib::ID3v2::Tag* tag, const QString& cmt) {
  if (tag->isEmpty()) return; // ID3v1 doesn't support iTunes comments
  if (tag->frameListMap().contains(keyId3TagiTunesComment)) {
    const TagLib::ID3v2::FrameList &comments = tag->frameListMap()[keyId3TagiTunesComment];
    bool found = false;
    for (TagLib::ID3v2::FrameList::ConstIterator it = comments.begin(); it != comments.end(); it++) {
      // overwrite all appropriate comment elements
      TagLib::ID3v2::CommentsFrame *commFrame = dynamic_cast<TagLib::ID3v2::CommentsFrame *>(*it);
      if (commFrame && commFrame->description().isEmpty()) {
        commFrame->setLanguage(lngId3TagiTunesComment);
        commFrame->setText(TagLib::String(cmt.toUtf8().constData(), TagLib::String::UTF8));
        // we don't save here, because MPEGs need v2.3 / 2.4 handling.
        found = true;
      }
    }
    if (found) return;
  }
  TagLib::ID3v2::CommentsFrame* frm = new TagLib::ID3v2::CommentsFrame();
  frm->setText(TagLib::String(cmt.toUtf8().constData(), TagLib::String::UTF8));
  frm->setLanguage(lngId3TagiTunesComment);
  tag->addFrame(frm);
  // again, don't save here.
  return;
}

bool MpegID3FileMetadata::setGrouping(const QString& grp) {
  setGroupingId3v2(specificFile->ID3v2Tag(), grp);
  specificFile->save(
    TagLib::MPEG::File::AllTags,
    true,
    specificFile->ID3v2Tag()->header()->majorVersion()
  );
  return true;
}


bool MpegID3FileMetadata::setGroupingId3v2(TagLib::ID3v2::Tag* tag, const QString& grp) {
  if (tag->isEmpty()) return false; // ID3v1 doesn't support Grouping
  TagLib::ID3v2::Frame* frm = new TagLib::ID3v2::TextIdentificationFrame(keyId3TagGrouping);
  frm->setText(TagLib::String(grp.toUtf8().constData(), TagLib::String::UTF8));
  tag->removeFrames(keyId3TagGrouping);
  tag->addFrame(frm);
  // again, don't save here
  return true;
}

bool MpegID3FileMetadata::setKey(const QString& key) {
  setKeyId3v2(specificFile->ID3v2Tag(), key);
  specificFile->save(
    TagLib::MPEG::File::ID3v2,
    false,
    specificFile->ID3v2Tag()->header()->majorVersion()
  );
  return true;
}

bool MpegID3FileMetadata::setKeyId3v2(TagLib::ID3v2::Tag* tag, const QString& key) {
  if (tag->isEmpty()) return false; // ID3v1 doesn't support Key
  TagLib::ID3v2::Frame* frm = new TagLib::ID3v2::TextIdentificationFrame(keyId3TagKey);
  frm->setText(TagLib::String(key.toUtf8().constData(), TagLib::String::UTF8));
  tag->removeFrames(keyId3TagKey);
  tag->addFrame(frm);
  // again, don't save in here
  return true;
}

// =================================== AIFF ====================================

QString AiffID3FileMetadata::getGrouping() const {
  return getGroupingId3v2(specificFile->tag());
}

QString AiffID3FileMetadata::getKey() const {
  return getKeyId3v2(specificFile->tag());
}

bool AiffID3FileMetadata::setComment(const QString& cmt) {
  // basic tag
  genericFile->tag()->setComment(
    TagLib::String(cmt.toUtf8().constData(), TagLib::String::UTF8)
  );
  // iTunes comment hack
  setITunesCommentId3v2(specificFile->tag(), cmt);
  specificFile->save();
  return true;
}

bool AiffID3FileMetadata::setGrouping(const QString& grp) {
  setGroupingId3v2(specificFile->tag(), grp);
  specificFile->save();
  return true;
}

bool AiffID3FileMetadata::setKey(const QString& key) {
  setKeyId3v2(specificFile->tag(), key);
  specificFile->save();
  return true;
}


// =================================== WAV =====================================

QString WavID3FileMetadata::getGrouping() const {
  return getGroupingId3v2(specificFile->tag());
}

QString WavID3FileMetadata::getKey() const {
  return getKeyId3v2(specificFile->tag());
}

bool WavID3FileMetadata::setComment(const QString& cmt) {
  genericFile->tag()->setComment(
    TagLib::String(cmt.toUtf8().constData(), TagLib::String::UTF8)
  );
  genericFile->save();
  return true;
}

// =================================== MP4 =====================================

QString Mp4FileMetadata::getGrouping() const {
  if (!specificFile->tag()->itemListMap().contains(keyMp4TagGrouping))
    return emptyString;
  TagLib::MP4::Item m = specificFile->tag()->itemListMap()[keyMp4TagGrouping];
  TagLib::String out = m.toStringList().front();
  return QString::fromUtf8((out.toCString(true)));
}

QString Mp4FileMetadata::getKey() const {
  if (!specificFile->tag()->itemListMap().contains(keyMp4TagKey))
    return emptyString;
  TagLib::MP4::Item m = specificFile->tag()->itemListMap()[keyMp4TagKey];
  TagLib::String out = m.toStringList().front();
  return QString::fromUtf8((out.toCString(true)));
}

bool Mp4FileMetadata::setGrouping(const QString& grp) {
  TagLib::StringList sl(TagLib::String(grp.toUtf8().constData(), TagLib::String::UTF8));
  specificFile->tag()->itemListMap().insert(keyMp4TagGrouping, sl);
  specificFile->save();
  return true;
}

bool Mp4FileMetadata::setKey(const QString& key) {
  TagLib::StringList sl(TagLib::String(key.toUtf8().constData(), TagLib::String::UTF8));
  specificFile->tag()->itemListMap().insert(keyMp4TagKey, sl);
  specificFile->save();
  return true;
}

// =================================== ASF =====================================

QString AsfFileMetadata::getGrouping() const {
  if (!specificFile->tag()->attributeListMap().contains(keyAsfTagGrouping))
    return emptyString;
  TagLib::ASF::AttributeList l = specificFile->tag()->attributeListMap()[keyAsfTagGrouping];
  TagLib::String out = l.front().toString();
  return QString::fromUtf8((out.toCString(true)));
}

QString AsfFileMetadata::getKey() const {
  if (!specificFile->tag()->attributeListMap().contains(keyAsfTagKey))
    return emptyString;
  TagLib::ASF::AttributeList l = specificFile->tag()->attributeListMap()[keyAsfTagKey];
  TagLib::String out = l.front().toString();
  return QString::fromUtf8((out.toCString(true)));
}

bool AsfFileMetadata::setGrouping(const QString& grp) {
  specificFile->tag()->setAttribute(
    keyAsfTagGrouping,
    TagLib::String(grp.toUtf8().constData(), TagLib::String::UTF8)
  );
  specificFile->save();
  return true;
}

bool AsfFileMetadata::setKey(const QString& key) {
  specificFile->tag()->setAttribute(
    keyAsfTagKey,
    TagLib::String(key.toUtf8().constData(), TagLib::String::UTF8)
  );
  specificFile->save();
  return true;
}


// =================================== APE =====================================

QString ApeFileMetadata::getGrouping() const {
  TagLib::APE::Tag* tag = dynamic_cast<TagLib::APE::Tag*>(genericFile->tag());
  if (!tag->itemListMap().contains(keyApeTagGrouping))
    return emptyString;
  TagLib::APE::Item m = tag->itemListMap()[keyApeTagGrouping];
  TagLib::String out = m.toStringList().front();
  return QString::fromUtf8((out.toCString(true)));
}

bool ApeFileMetadata::setGrouping(const QString& grp) {
  TagLib::APE::Tag* tagTestApe = dynamic_cast<TagLib::APE::Tag*>(specificFile->tag());
  tagTestApe->addValue(
    keyApeTagGrouping,
    TagLib::String(grp.toUtf8().constData(), TagLib::String::UTF8)
  );
  specificFile->save();
  return true;
}
