import { Component, OnInit } from '@angular/core';
import { AngularFireDatabase, AngularFireList } from '@angular/fire/compat/database';
import { Observable } from 'rxjs';
@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})


// export class AppComponent {
//   title = 'projet';
//   items: Observable<any[]>;
//   constructor(db: AngularFireDatabase) {
//     this.items = db.list('items').valueChanges();
//   }
// }

export class AppComponent implements OnInit {
  title = 'projet';
  itemRef: AngularFireList<any>;
  items?: Observable<any[]>;
  constructor(db: AngularFireDatabase) {
    this.itemRef = db.list('items');
  }
  ngOnInit(): void{
    this.items = this.itemRef.valueChanges();
  }
}


