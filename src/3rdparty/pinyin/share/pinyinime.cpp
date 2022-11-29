/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include "pinyinime.h"
#include "dicttrie.h"
#include "matrixsearch.h"
#include "spellingtrie.h"

#ifdef __cplusplus
extern "C" {
#endif

  using namespace ime_pinyin;

  // The maximum number of the prediction items.
  static const size_t kMaxPredictNum = 500;

  // Used to search Pinyin string and give the best candidate.
  typedef class PinyinIME{
  public:
      MatrixSearch*matrix_search;
      char16 predict_buf[kMaxPredictNum][kMaxPredictSize + 1];
      PinyinIME(){
         matrix_search = new MatrixSearch();
      }
      ~PinyinIME(){
         delete matrix_search;
      }
  }PINYINIME;

  void* im_open_decoder(const char *fn_sys_dict, const char *fn_usr_dict) {
      PINYINIME*ime=new PINYINIME;
      ime->matrix_search->init(fn_sys_dict, fn_usr_dict);
      return ime;
  }

  bool im_open_decoder_fd(int sys_fd, long start_offset, long length,
                          const char *fn_usr_dict) {
    /*if (NULL != matrix_search)
      delete matrix_search;

    matrix_search = new MatrixSearch();
    if (NULL == matrix_search)
      return false;

    return matrix_search->init_fd(sys_fd, start_offset, length, fn_usr_dict);*/return true;
  }

  void im_close_decoder(void*hime) {
      delete (PINYINIME*)hime;
  }

  void im_set_max_lens(void*hime,size_t max_sps_len, size_t max_hzs_len) {
      PINYINIME*ime=(PINYINIME*)hime;
      ime->matrix_search->set_max_lens(max_sps_len, max_hzs_len);
  }

  void im_flush_cache(void*hime) {
      ((PINYINIME*)hime)->matrix_search->flush_cache();
  }

  // To be updated.
  size_t im_search(void*hime,const char* pybuf, size_t pylen) {
      PINYINIME*ime=(PINYINIME*)hime;
      ime->matrix_search->search(pybuf, pylen);
      return ime->matrix_search->get_candidate_num();
  }

  size_t im_delsearch(void*hime,size_t pos, bool is_pos_in_splid,
                      bool clear_fixed_this_step) {
      PINYINIME*ime=(PINYINIME*)hime;
      ime->matrix_search->delsearch(pos, is_pos_in_splid, clear_fixed_this_step);
      return ime->matrix_search->get_candidate_num();
  }

  void im_reset_search(void*hime) {
      PINYINIME*ime=(PINYINIME*)hime;
      ime->matrix_search->reset_search();
  }

  // To be removed
  size_t im_add_letter(char ch) {
      return 0;
  }

  const char* im_get_sps_str(void*hime,size_t *decoded_len) {
      PINYINIME*ime=(PINYINIME*)hime;
      return ime->matrix_search->get_pystr(decoded_len);
  }

  char16* im_get_candidate(void*hime,size_t cand_id, char16* cand_str,
                        size_t max_len) {
      PINYINIME*ime=(PINYINIME*)hime;
      return ime->matrix_search->get_candidate(cand_id, cand_str, max_len);
  }

  size_t im_get_spl_start_pos(void*hime,const uint16 *&spl_start) {
      PINYINIME*ime=(PINYINIME*)hime;
      return ime->matrix_search->get_spl_start(spl_start);
  }

  size_t im_choose(void*hime,size_t choice_id) {
      PINYINIME*ime=(PINYINIME*)hime;
      return ime->matrix_search->choose(choice_id);
  }

  size_t im_cancel_last_choice(void*hime) {
      PINYINIME*ime=(PINYINIME*)hime;
      return ime->matrix_search->cancel_last_choice();
  }

  size_t im_get_fixed_len(void*hime) {
      PINYINIME*ime=(PINYINIME*)hime;
      return ime->matrix_search->get_fixedlen();
  }

  // To be removed
  bool im_cancel_input() {
     return true;
  }


  size_t im_get_predicts(void*hime,const char16 *his_buf,
                         char16 (*&pre_buf)[kMaxPredictSize + 1]) {
      PINYINIME*ime=(PINYINIME*)hime;
      if (NULL == his_buf)
         return 0;
      size_t fixed_len = utf16_strlen(his_buf);
      const char16 *fixed_ptr = his_buf;
      if (fixed_len > kMaxPredictSize) {
         fixed_ptr += fixed_len - kMaxPredictSize;
         fixed_len = kMaxPredictSize;
      }

      pre_buf = ime->predict_buf;
      return ime->matrix_search->get_predicts(his_buf, pre_buf, kMaxPredictNum);
  }

  void im_enable_shm_as_szm(bool enable) {
      SpellingTrie &spl_trie = SpellingTrie::get_instance();
      spl_trie.szm_enable_shm(enable);
  }

  void im_enable_ym_as_szm(bool enable) {
      SpellingTrie &spl_trie = SpellingTrie::get_instance();
      spl_trie.szm_enable_ym(enable);
  }

#ifdef __cplusplus
}
#endif
