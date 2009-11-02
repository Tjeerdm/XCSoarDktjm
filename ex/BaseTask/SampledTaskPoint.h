/* Generated by Together */

#ifndef SAMPLEDTASKPOINT_H
#define SAMPLEDTASKPOINT_H

#include "Navigation/SearchPointVector.hpp"
#include "Scoring/ObservationZone.hpp"
#include "TaskPoint.hpp"
#include "Navigation/TaskProjectionClient.hpp"

class SampledTaskPoint:
  public TaskPoint, 
  public ObservationZone,
  public TaskProjectionClient
{
public:  
  SampledTaskPoint(const TaskProjection& tp,
                   const Waypoint & wp, 
                   const bool b_scored):
    TaskProjectionClient(tp),
    TaskPoint(wp),
    boundary_scored(b_scored),
    search_max(getLocation(),tp),
    search_min(getLocation(),tp)
  {
    clear_boundary_points();
    clear_sample_points();
  };

    virtual ~SampledTaskPoint() {};

  const SearchPointVector& get_search_points(bool cheat=false);

  const SearchPointVector& get_boundary_points() const;

  virtual bool prune_boundary_points();

  virtual bool prune_sample_points();

  virtual void update_projection();

  virtual void clear_boundary_points();

  virtual void clear_sample_points();

  virtual void clear_sample_all_but_last(const AIRCRAFT_STATE&);

  void set_search_max(const SearchPoint &i) {
    search_max = i;
  }

  void set_search_min(const SearchPoint &i) {
    search_min = i;
  }

  const SearchPoint& get_search_max() const {
    return search_max;
  }

  const SearchPoint& get_search_min() const {
    return search_min;
  }

  virtual void default_boundary_points();

  GEOPOINT getMaxLocation() const {
    return search_max.getLocation();
  };
  GEOPOINT getMinLocation() const {
    return search_min.getLocation();
  };

  virtual bool update_sample(const AIRCRAFT_STATE&);

#ifdef DO_PRINT
  virtual void print(std::ostream& f, const AIRCRAFT_STATE&state) const;
  virtual void print_samples(std::ostream& f, const AIRCRAFT_STATE&state);
  virtual void print_boundary(std::ostream& f, const AIRCRAFT_STATE&state) const;
#endif

protected:
  bool boundary_scored;

private:
  SearchPointVector sampled_points;
  SearchPointVector boundary_points;
  SearchPoint search_max;
  SearchPoint search_min;
};
#endif //SAMPLEDOBSERVATIONZONE_H
